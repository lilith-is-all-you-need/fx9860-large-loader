"""Bind the boot program to exact immutable payload bytes before compilation."""
import argparse,hashlib,json,struct
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('raw',type=Path);p.add_argument('out',type=Path)
a=p.parse_args();root=Path(__file__).resolve().parents[1]
raw=a.raw.read_bytes();header=bytearray(512);header[:16]=b'KHICAS-MAPPED/1\0'
struct.pack_into('>III',header,16,1,0x00300200,len(raw))
image=header+raw;digest=hashlib.sha256(image).digest()
if len(image)>3000000:raise SystemExit('payload exceeds current storage budget')
a.out.mkdir(parents=True,exist_ok=True);(a.out/'KHICAS.BIN').write_bytes(image)
(root/'src/expected.h').write_text('#ifndef KHIMAP_EXPECTED_H\n#define KHIMAP_EXPECTED_H\n#define IMAGE_BYTES '+str(len(image))+'u\nstatic const unsigned char image_sha256[32]={'+','.join(hex(b) for b in digest)+'};\n#endif\n',encoding='ascii')
report={'raw':str(a.raw.resolve()),'rawSha256':hashlib.sha256(raw).hexdigest(),'imageBytes':len(image),'pages':(len(image)+4095)//4096,'imageSha256':digest.hex(),'entry':'0x00300200','base':'0x00300000','headerBytes':512}
(a.out/'payload.json').write_text(json.dumps(report,indent=2),encoding='utf8');print(json.dumps(report))
