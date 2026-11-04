"""Fresh host-process tests of the actual G11 card startup/transport bodies.

No QNX SDK, target code execution, vehicle or phone is used. A native ELF core
and its MOST receiver remain separate physical-acceptance requirements.
"""
from pathlib import Path
import os,subprocess,tempfile,zlib

REPO=Path(__file__).resolve().parents[1]

def saved(position):
    body=f'format=1\nposition={position}\ngeneration=42\n'.encode('ascii')
    return body+f'crc32={zlib.crc32(body):x}\n'.encode('ascii')

def main():
    records=[]
    with tempfile.TemporaryDirectory(prefix='cpmap-card-startup-') as directory:
        tmp=Path(directory)
        for firmware in (1433,1447):
            program=tmp/f'card-startup-{firmware}'
            command=['/usr/bin/cc','-B/usr/bin/','-std=gnu99','-O1','-g','-Wall','-Wextra','-Werror',
                '-Wno-unused-function','-fno-pie','-no-pie','-fsanitize=address,undefined',
                f'-DCPMAP_FIRMWARE={firmware}','-DCPMAP_CLUSTER=790',
                '-include',str(REPO/'src/card-core/source/profile_card.h'),
                str(REPO/'src/card-core/tests/test_card_startup790.c'),'-ldl','-pthread','-o',str(program)]
            subprocess.run(command,check=True,timeout=90)
            env=dict(os.environ,ASAN_OPTIONS='handle_segv=0:halt_on_error=1:detect_leaks=1')
            for position in range(4):
                for mode in ('existing','late','previous'):
                    root=tmp/f'{firmware}-{position}-{mode}';root.mkdir()
                    primary=root/'navigation-card.state';previous=root/'navigation-card.previous'
                    expected=saved(position)
                    if mode=='existing':primary.write_bytes(expected)
                    elif mode=='previous':primary.write_bytes(b'invalid\n');previous.write_bytes(expected)
                    before={p.name:p.read_bytes() for p in root.iterdir()}
                    result=subprocess.run([str(program),str(root),str(position),mode],
                        capture_output=True,text=True,timeout=15,env=env)
                    assert result.returncode==0,(firmware,position,mode,result.stdout,result.stderr)
                    assert 'CARD_STARTUP_TRANSPORT_PASS' in result.stdout
                    after={p.name:p.read_bytes() for p in root.iterdir()}
                    if mode=='late':assert after=={'navigation-card.state':expected}
                    else:assert before==after
                    record=dict(firmware=firmware,position=position,mode=mode,
                        fresh_host_process=True,asan_ubsan_pass=True,
                        startup_and_reconnect_reapplied=True,ui_reselection_required=False)
                    records.append(record);print(result.stdout.strip(),flush=True)
    print('G11_CARD_STARTUP_TRANSPORT_ALL_PASS cases='+str(len(records))+' physical_acceptance=0',flush=True)
    return records

if __name__=='__main__':main()
