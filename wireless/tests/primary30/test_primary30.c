#include "primary30_mock.h"
#include <assert.h>
#include <stdio.h>

static Obj *display(int w,int h,int fps,int type)
{
    Obj *d=mdict();mput(d,"widthPixels",mnum(w));mput(d,"heightPixels",mnum(h));
    mput(d,"maxFPS",mnum(fps));if(type>=0)mput(d,"type",mnum(type));
    mput(d,"uuid",mstr("unchanged-display-id"));return d;
}
static Obj *fixture(int mainfps,int main_type,int duplicates,int reverse)
{
    Obj *root=mdict(),*a=marray(),*main=display(1264,640,mainfps,main_type);
    Obj *alt=display(800,480,30,111);
    if(reverse){mappend(a,alt);mappend(a,main);}else{mappend(a,main);mappend(a,alt);}
    if(duplicates)mappend(a,display(1264,640,60,-1));
    mput(root,"displays",a);mput(root,"features",mnum(123456));
    mput(root,"keepNested",display(42,43,44,-1));mfreeze(root);return root;
}
int main(void)
{
    V74CF api=mapi(),missing;
    Obj *root,*copy,*a,*original_primary;int rc,i,order,type;
    unsigned cases=0;
    for(order=0;order<2;++order)for(type=0;type<2;++type){
        root=fixture(60,type?110:-1,0,order);a=mget(root,"displays");
        original_primary=(Obj *)api.item(a,order?1:0);
        copy=v74_primary30_copy(&api,root,1,1264,640,&rc);
        assert(copy && copy!=root && rc==V74_PRIMARY_CHANGED);
        assert(mvalue(original_primary,"maxFPS")==60);
        assert(mvalue(api.item(mget(copy,"displays"),order?1:0),"maxFPS")==30);
        assert(mget(copy,"keepNested")==mget(root,"keepNested"));
        assert(mget(copy,"features")==mget(root,"features"));
        assert(api.item(mget(copy,"displays"),order?0:1)==api.item(a,order?0:1));
        assert(mget(api.item(mget(copy,"displays"),order?1:0),"uuid")==mget(original_primary,"uuid"));
        assert(!v74_primary30_copy(&api,copy,1,1264,640,&rc) && rc==V74_PRIMARY_ALREADY30);
        mrelease(copy);mrelease(root);assert(mlive()==0);++cases;
    }
    for(i=0;i<14;++i){
        root=fixture(60,-1,0,0);mfail_after(i);
        copy=v74_primary30_copy(&api,root,1,1264,640,&rc);
        mfail_after(-1);if(copy)mrelease(copy);
        assert(mvalue(api.item(mget(root,"displays"),0),"maxFPS")==60);
        mrelease(root);assert(mlive()==0);++cases;
    }
    root=fixture(60,-1,0,0);
    missing=api;missing.array_set=NULL;
    assert(!v74_primary30_copy(&missing,root,1,1264,640,&rc) && rc==V74_PRIMARY_INVALID);++cases;
    assert(!v74_primary30_copy(&api,root,0,1264,640,&rc) && rc==0);++cases;
    assert(!v74_primary30_copy(&api,root,1,800,480,&rc) && rc==0);++cases;
    assert(!v74_primary30_copy(&api,root,1,0,640,&rc) && rc==V74_PRIMARY_INVALID);++cases;
    mrelease(root);assert(mlive()==0);
    for(i=0;i<4;++i){
        root=fixture(i==0?25:(i==1?30:60),i==2?111:-1,i==3,0);
        assert(!v74_primary30_copy(&api,root,1,1264,640,&rc));
        mrelease(root);assert(mlive()==0);++cases;
    }
    root=mdict();mput(root,"displays",mstr("bad-type"));mfreeze(root);
    assert(!v74_primary30_copy(&api,root,1,1264,640,&rc));mrelease(root);++cases;
    root=mdict();a=marray();mappend(a,mnum(123));mput(root,"displays",a);mfreeze(root);
    assert(!v74_primary30_copy(&api,root,1,1264,640,&rc));mrelease(root);++cases;
    for(i=0;i<2;i++){
        Obj *main,*alt;
        root=mdict();a=marray();main=display(i?800:1264,i?480:640,i?50:60,110);
        alt=display(800,480,i?40:60,111);mappend(a,alt);mappend(a,main);mput(root,"displays",a);mfreeze(root);
        copy=v74_primary30_copy(&api,root,1,0,0,&rc);
        assert(copy&&rc==V74_PRIMARY_CHANGED);
        assert(api.item(mget(copy,"displays"),0)==alt&&mvalue(alt,"maxFPS")== (i?40:60));
        assert(mvalue(api.item(mget(copy,"displays"),1),"maxFPS")==30);
        mrelease(copy);mrelease(root);assert(mlive()==0);++cases;
    }
    root=fixture(60,-1,0,0);
    assert(!v74_primary30_copy(&api,root,1,0,0,&rc));mrelease(root);++cases;
    assert(mlive()==0);printf("PRIMARY30_CF_COPY_TEST_PASS cases=%u original_mutations=0 leaks=0 measured_fps=0\n",cases);
    return 0;
}
