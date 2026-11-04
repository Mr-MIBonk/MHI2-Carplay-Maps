"""Host-shell argument/config tests. No QNX library or car is executed."""
import argparse
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

REPO=Path(__file__).resolve().parents[1]
SOURCE=REPO/'src/information/runtime/mm-ipod.wrapper.sh'

def shell_path(path):
    value=str(path.resolve()).replace('\\','/')
    if os.name=='nt' and len(value)>2 and value[1]==':':
        value='/'+value[0].lower()+value[2:]
    return value

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--shell',default='sh')
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='cpmap-wrapper-') as tmp:
        root=Path(tmp)
        runtime=root/'runtime';runtime.mkdir()
        feature=root/'feature';(feature/'stock_first').mkdir(parents=True)
        (feature/'information').mkdir()
        capture=root/'args.txt'
        real=feature/'stock_first/mm-ipod.real'
        real.write_text('#!/bin/sh\n: > "$CAPTURE"\nfor arg; do printf "%s\\n" "$arg" >> "$CAPTURE"; done\n',encoding='utf-8')
        real.chmod(0o755)
        cfg=root/'original-iap2.cfg'
        script=root/'wrapper.sh'
        text=SOURCE.read_text(encoding='utf-8')
        text=text.replace('ROOT=/mnt/app/carplaymaps','ROOT='+shlex.quote(shell_path(feature)))
        text=text.replace('/ramdisk/cpmap-info',shell_path(runtime))
        text=text.replace('CFG=/etc/mm/iap2.cfg','CFG='+shlex.quote(shell_path(cfg)))
        script.write_text(text,encoding='utf-8')
        environment={**os.environ,'CAPTURE':shell_path(capture),'MSYS2_ARG_CONV_EXCL':'*'}
        def run(options):
            subprocess.run([args.shell,shell_path(script),*options],env=environment,check=True,capture_output=True,text=True,timeout=15)
            return capture.read_text().splitlines()
        original=['-d','iap2,config=/etc/mm/iap2.cfg,probe','--keep','value with spaces']
        assert run(original)==original  # disabled: no change
        (feature/'information/ENABLED').touch()
        assert run(original)==original  # missing config: no destructive shift
        cfg.write_text('[ident]\nsendmsgs=0x5200,0x5203\nrecvmsgs=0x5201,0x5202\n')
        expected=original.copy();expected[1]=expected[1].replace('/etc/mm/iap2.cfg',shell_path(runtime/'iap2.cfg'))
        assert run(original)==expected
        assert (runtime/'iap2.cfg').read_text().count('0x5204')==1
        assert '0x5204' not in cfg.read_text()  # stock config stays byte-identical
        unknown=['-d','iap2,config=/another/driver.cfg','--keep','A B']
        assert run(unknown)==unknown
        assert run([])==[]
        cfg.write_text('[ident]\nrecvmsgs=0x5201,0x5202,0x5204\n')
        assert run(original)==expected
        assert (runtime/'iap2.cfg').read_text()==cfg.read_text()
    # Parse actual unmodified sources with the chosen shell as well.
    scripts=[* (REPO/'src/information/runtime').glob('*.sh'),
             REPO/'src/toolbox/install-carplay-maps.sh',
             REPO/'src/toolbox/restore-carplay-maps.sh',
             REPO/'src/toolbox/dio_manager.wrapper.sh']
    for script in scripts:
        subprocess.run([args.shell,'-n',shell_path(script)],check=True,capture_output=True,text=True,timeout=15)
    print('INFORMATION_WRAPPER_HOST_PASS disabled=1 missing_config=1 known_rewrite=1 unknown_args_preserved=1 no_args=1 stock_config_unchanged=1 syntax_sources='+str(len(scripts))+' real_vehicle=0')
if __name__=='__main__':main()
