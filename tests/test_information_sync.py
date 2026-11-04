"""Deterministic synthetic logs, not real vehicle evidence."""
from pathlib import Path
import subprocess
import sys
import tempfile

fixtures=Path(__file__).resolve().parents[1]/'src/information/tests'
cases={
 'v521-association.synthetic.log':{'man_index':'2','turn_to':'Road B','lane_idx':'7','lane_count':'3'},
 'v532-maneuver-fallback.synthetic.log':{'man_index':'2','lane_idx':'2','lane_count':'3'},
 'v532-route-zero-debounce.synthetic.log':{'state':'1','road':'Stable Road','lane_idx':'7','lane_count':'3'},
 'v532-hard-clear.synthetic.log':{'state':'0','road':'','lane_count':'0'},
}
for name,expected in cases.items():
 with tempfile.TemporaryDirectory(prefix='cpmap-replay-') as tmp:
  out=Path(tmp)/'cache.pps'
  subprocess.run([sys.argv[1],'--replay',str(fixtures/name),str(out)],check=True,timeout=15)
  actual=dict(line.split('::',1) for line in out.read_text().splitlines() if '::' in line)
  for field,value in expected.items():
   assert actual[field]==value,(name,field,actual.get(field),value)
  print('INFORMATION_SYNTHETIC_REPLAY_PASS',name,flush=True)
