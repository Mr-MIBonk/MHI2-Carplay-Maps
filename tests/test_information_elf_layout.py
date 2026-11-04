"""Read-only exact-firmware table/relocation audit. OEM inputs stay private.

Requires pyelftools. This establishes native layout invariants, NOT a live
phone session, complete calling-convention proof or on-unit acceptance.
"""
from pathlib import Path
import argparse,hashlib,json,struct
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

MUS=('MU1102','MU1367','MU1440','MU1433','MU1447')
REQUIRED=('packet_sendlist_pop','packet_sendlist_push','packet_addparam_blob','link_send_message')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--fixtures',type=Path,required=True)
 p.add_argument('--output',type=Path);a=p.parse_args();results=[]
 for mu in MUS:
  file=a.fixtures/mu/'armle/lib/dll/ipod-drvr-iap2.so'
  with file.open('rb') as stream:
   elf=ELFFile(stream);assert elf.elfclass==32 and elf.little_endian and elf['e_machine']=='EM_ARM'
   # The original callback can be local (not a dlsym export). Read its retained
   # static symbol too, without treating an arbitrary non-NULL pointer as proof.
   symbols={}
   for table in (elf.get_section_by_name('.symtab'),elf.get_section_by_name('.dynsym')):
    if table is not None:symbols.update({s.name:s for s in table.iter_symbols()})
   ident=symbols['ident_info_funcs'];family=symbols['iap2_ctrl_msg_table']
   assert ident['st_size']==24*8 and family['st_size']==256*12,(mu,'table size mismatch')
   def read(address,count):
    for segment in elf.iter_segments():
     if segment['p_type']=='PT_LOAD' and segment['p_vaddr']<=address and address+count<=segment['p_vaddr']+segment['p_filesz']:
      stream.seek(segment['p_offset']+address-segment['p_vaddr']);return stream.read(count)
    raise AssertionError('Address is not file-backed')
   entries=read(ident['st_value'],ident['st_size'])
   indices=[i for i in range(24) if struct.unpack_from('<I',entries,i*8)[0]==17]
   assert len(indices)==1,(mu,'id17 missing/duplicate')
   pointer_address=ident['st_value']+indices[0]*8+4
   value=struct.unpack_from('<I',entries,indices[0]*8+4)[0]
   relocation_names=[]
   for section in elf.iter_sections():
    if not isinstance(section,RelocationSection):continue
    for relocation in section.iter_relocations():
     if relocation['r_offset']==pointer_address and relocation['r_info_sym']:
      relocation_names.append(elf.get_section(section['sh_link']).get_symbol(relocation['r_info_sym']).name)
   direct=[s.name for s in symbols.values() if s['st_value']==value and s['st_info']['type']=='STT_FUNC']
   callbacks=set(relocation_names+direct)
   assert 'ident_info_tspbt' in callbacks,(mu,'stock callback differs',sorted(callbacks))
   row=struct.unpack('<III',read(family['st_value']+0x52*12,12))
   assert row[1:]==(0,0),(mu,'native family 0x52 unexpectedly populated')
   functions={}
   for name in REQUIRED:
    symbol=symbols[name];assert symbol['st_info']['type']=='STT_FUNC' and symbol['st_size']>=16
    functions[name]={'bytes':symbol['st_size'],'sha256':hashlib.sha256(read(symbol['st_value']&~1,symbol['st_size'])).hexdigest()}
   callback=symbols['ident_info_tspbt']
   callback_hash=hashlib.sha256(read(callback['st_value']&~1,callback['st_size'])).hexdigest()
   results.append(dict(firmware=mu,driver_sha256=hashlib.sha256(file.read_bytes()).hexdigest(),ident_entries=24,ident_id17_index=indices[0],stock_callback='ident_info_tspbt',stock_callback_sha256=callback_hash,message_families=256,family52_empty=True,functions=functions,full_abi_or_real_session_proven=False))
 # These are the four exact OEM functions called by the public native hook.
 # Require their machine code to match the known MU1102 reference, rather than
 # assuming that a same-named export alone proves calling-convention parity.
 for name in REQUIRED:
  assert len({r['functions'][name]['sha256'] for r in results})==1,('Changed OEM function body',name)
 report={'firmwares':results,'four_called_oem_functions_byte_identical':True,'original_firmware_modified':False,'vehicle_acceptance':False}
 if a.output:
  assert not a.output.exists();a.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print('INFORMATION_EXACT_ELF_LAYOUT_PASS targets='+str(len(results))+' id17_stock_callback=5 family52_empty=5 ident_entry_bytes=8 family_entry_bytes=12 called_oem_function_bodies_identical=4 real_session=not_proven')
if __name__=='__main__':main()
