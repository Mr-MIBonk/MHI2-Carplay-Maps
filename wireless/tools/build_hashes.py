"""Generate coupled receiver headers from supplied frozen inputs, not a vehicle."""
from pathlib import Path
import hashlib,sys
sd=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]).resolve()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
transport=sd/'carplay/payload/wireless/common/runtime/libv75-transport.so'
for profile,cluster in [('AID12.3','791'),('AID10.5-17PM','790')]:
    d=out/profile;d.mkdir()
    values={'OMONOB_RECEIVER_SHA':sha(sd/'carplay/payload/wireless/profiles'/profile/'wcp_receiver'),
            'OMONOB_INTERFACE_SHA':sha(out/cluster/'libomonob-wireless-maps.so'),
            'OMONOB_TRANSPORT_SHA':sha(transport),'OMONOB_CLUSTER':cluster}
    (d/'maps_hashes.h').write_text(''.join('#define %s "%s"\n'%v for v in values.items()),encoding='ascii')
