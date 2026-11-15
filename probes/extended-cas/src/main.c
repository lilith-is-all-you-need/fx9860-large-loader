#include "loader.h"
#include "sha256.h"
#include "os-map-profile.h"
static uint8_t rd(void *p,uint32_t a){(void)p;return BYTE(a);}
static void wide(uint16_t *out,const char *p){while(*p)*out++=(unsigned char)*p++;*out=0;}
static void line(int y,const char *s){r_print(0,y,(const unsigned char *)s,0);}
static void hexline(int y,const char *label,uint32_t v)
{
 char buf[22];unsigned n=0;while(label[n]&&n<12){buf[n]=label[n];n++;}
 for(int i=7;i>=0;i--){unsigned x=(v>>(i*4))&15;buf[n++]=(char)(x<10?'0'+x:'A'+x-10);}buf[n]=0;line(y,buf);
}
static int scan(Context *c)
{
 FxOsMapProfile os;int rc=fx_os_profile(rd,0,&os);if(rc)return 100+rc;
 uint16_t path[32];wide(path,"\\\\fls0\\KHICAS.BIN");
 int fd=r_open(path,1,0);if(fd<0)return 10;
 if(r_size(fd)!=(int)IMAGE_BYTES){r_close(fd);return 11;}
 Sha256 sha;unsigned char digest[32];sha256_init(&sha);
 for(unsigned p=0;p<PAGE_COUNT;p++){
  uint32_t pa=0;int span=((int(*)(int,int,uint32_t *))(uintptr_t)os.query)(fd,(int)(p*4096),&pa);
  pa&=0x1fffffffu;
  if(span!=4096||(pa&4095)||pa<0x10000||pa>0x1fff000u){r_close(fd);return 12;}
  c->pages[p]=pa;unsigned bytes=IMAGE_BYTES-p*4096;if(bytes>4096)bytes=4096;
  sha256_update(&sha,(const volatile unsigned char *)(uintptr_t)(pa|0xa0000000u),bytes);
  /* Payload P0 pages use C=0; no virtual/physical cache synonym assumption. */
 }
 sha256_final(&sha,digest);
 if(r_close(fd)<0)return 13;
 for(unsigned i=0;i<32;i++)if(digest[i]!=image_sha256[i])return 14;
 BARRIER();return 0;
}
/* Used only after payload finalizers and resident heap have been released. */
static int menu_timer;
static void menu_callback(void)
{r_putkey(4,9,0);r_timer_stop(menu_timer);r_timer_remove(menu_timer);}
static int menu(void)
{
 r_clear_keys();menu_timer=r_timer_install(0,menu_callback,0);
 if(menu_timer<=0)return 0;
 r_timer_start(menu_timer);int col=0,row=0;uint16_t key=0;
 r_keywait(&col,&row,0,1,0,&key);
 r_timer_stop(menu_timer);r_timer_remove(menu_timer);return 1;
}
int main(void)
{
 for(;;){
  r_clear();line(0,"KhiCAS mapped RC1");line(16,"Checking KHICAS.BIN");r_display();
  unsigned n=resident_end-resident_begin,bytes=(sizeof(Context)+255+n+31)&~31u;
  Context *c=r_malloc(bytes);int result=0;unsigned reason=0;
  if(!c)result=20;
  if(c){
   for(unsigned i=0;i<sizeof *c;i++)((unsigned char *)c)[i]=0;
   c->resident_bytes=n;c->allocation_bytes=bytes;
   result=scan(c);
   if(!result){
    uintptr_t runner=((uintptr_t)c+sizeof(Context)+255)&~255u,uncached=runner|0x20000000u;
    for(unsigned i=0;i<n;i+=32)purge(runner+i);
    BARRIER();
    for(unsigned i=0;i<n;i++)BYTE(uncached+i)=resident_begin[i];
    if(resident_fixups[0]>511)result=21;
    for(unsigned i=1;!result&&i<=resident_fixups[0];i++){
     uint32_t off=resident_fixups[i];
     if(off+4>n||(off&3)){result=22;break;}
     uint32_t v=REG(uncached+off);
     if(v<(uintptr_t)resident_begin||v>=(uintptr_t)resident_end){result=23;break;}
     REG(uncached+off)=v-(uintptr_t)resident_begin+uncached;
    }
    BARRIER();
    if(!result){
     line(32,"Starting CAS...");r_display();
     int (*run)(Context *)=(int(*)(Context *))(uncached+(uintptr_t)resident_main-(uintptr_t)resident_begin);
     result=run(c);reason=c->reason;
     if(c->failure_stage)result=40;
    }
   }
   if(result){r_clear();line(0,"KhiCAS start failed");hexline(16,"Error ",result);hexline(32,"Stage ",c->failure_stage?c->failure_stage:c->stage);hexline(48,"Status ",c->failure_stage?c->failure_status:c->status);r_display();}
   r_free(c);
  }else{line(32,"Not enough memory");r_display();}
  if(result){int key=0;while(key!=30002&&key!=30010)r_getkey(&key);}
  if(reason==2)r_poweroff();
  if(!menu())return 1;
 }
}
