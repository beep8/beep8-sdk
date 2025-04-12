pico-8 cartridge // http://www.pico-8.com
version 14
__lua__
function _init()
anc, ane, anb
= new_array(), new_array(), new_array()
asw( anc,awr)
apb(anc,0)
apb(anc,0)
apb(anc,0)
apb(anc,0)
apb(anc,1)
apb(anc,2)
apb(anc,1)
asw( ane,awr)
apb(ane,0)
apb(ane,0)
apb(ane,0)
apb(ane,0)
apb(ane,0)
apb(ane,0)
apb(ane,0)
apb(ane,1)
apb(ane,2)
apb(ane,1)
asw( anb,awr)
local elm={idx=2,xf=false,yf=false}
apb(anb,elm)
elm.idx=1
apb(anb,elm)
elm.idx=0
apb(anb,elm)
elm.idx,elm.yf=1,true
apb(anb,elm)
elm.idx=2
apb(anb,elm)
elm.idx,elm.xf=1,true
apb(anb,elm)
elm.idx,elm.yf=0,false
apb(anb,elm)
elm.idx=1
apb(anb,elm)
income_reset()
_reset( ee.rrps )
end
_uid,ee = 0,{}
function enum(str)
local sp,word = 1,""
while true do
local chr = sub(str,sp,sp)
sp += 1
if chr == " " or chr == "\n" or chr == "" then
if word ~= "" then
ee[ word ] = _uid
_uid += 1
end
word = ""
else
word = word .. chr
end
if chr == "" then return end
end
end
gg = {}
function def_table( str )
local tbl,ctbl,sp,word,cnt,lhs={},nil,1,"",0,nil
while true do
local chr = sub(str,sp,sp)
sp+=1
if chr == "=" then
gg[ word ] = tbl
word=""
elseif chr == "]" then
ctbl,cnt = {},0
tbl[ ee[ word ] ] = ctbl
word=""
elseif chr == " " or chr == "}" then
if (cnt%2)==0 then
lhs = word
else
ctbl[ lhs ] = word
end
word=""
cnt+=1
elseif chr == "\n" or chr == "" then
else
word = word .. chr
end
if chr == "" then return end
end
return  tbl
end
function str2table( tbl_ , str_ )
local sp,word,lhs,isstr=1,"","",false
while true do
local chr = sub(str_,sp,sp)
sp+=1
if chr == "=" then
lhs = word
word = ""
elseif chr == "\n" then
if word == "true" then
tbl_[ lhs ] = true
elseif word == "false" then
tbl_[ lhs ] = false
else
if isstr == false then
tbl_[ lhs ] = tonum( word )
else
tbl_[ lhs ] = word
end
end
word = ""
isstr=false
else
if chr != "\"" then
word = word .. chr
else
isstr=true
end
end
if chr == "" then return tbl_ end
end
end
function flrshr(x,y)
return flr(shr(x,y))
end
pag,
paa,
pvk,
grv,
espe,
espe_kani
=
0.08,
0.08,
-2.6,
0.25,
0.25,
0.35
srw,
sro,
srd,
sre,
srec,
srfw,
sreg,
srpb
= 1,2,4,8,16,32,64,128
bgco,
bgsl,
bgtt,
bgkn,
bgcc,
bgbk,
bgex,
bgfw,
bgba,
bgdr,
bgtype_egg
= 100,101,102,103,104,105,106,107,108,109,110
spridx_0, spridx_s, spridx_e = 64,74,75
awa,
awr
= 200,201
local _estr=[[
prbg
prfw
prmp
prco
pren
ppc
ppe
pbbk
ptb
pti
ptel
pter
ets
ett
etk
ete
walltype_a
walltype_b
jstg
jskj
jswj
edpj
edpa
edsp
edpk
edgc
edso
edko
edbs
edko_penpen
edip
spsl
spsl_crushed
spci
sppp
sppp_stop
sppp_jump
sppp_slip
sppp_kick
spr_block_1x1
spr_block_1x1_broken
spr_concrete
spcg
spcbg
spcbr
spcic
sptcd
spr_turtle
spex
spgc
spci_empty
spr_kani
spka
spkc
spegg
spfwll
spba
spbb
spbc
spgd
spbel
spber
coin_idle
coin_bonus
coin_empty
ins_none
ins_coin
rrt
rrc
rrps
spr_pipe_top_0
spr_pipe_top_1
spr_pipe_stem_0
spr_pipe_stem_1
]]
enum(_estr)
def_table([[
sprites_ex=
spsl]spi 103 _rep 2 _w 1 _h 1}
spsl_crushed]spi 105 _rep 1 _w 1 _h 1}
sppp]spi 36 _rep 6 _w 2 _h 2}
sppp_stop]spi 14 _rep 1 _w 2 _h 2}
sppp_jump]spi 12 _rep 1 _w 2 _h 2}
sppp_slip]spi 76 _rep 2 _w 2 _h 2}
sppp_kick]spi 110 _rep 1 _w 2 _h 2}
spr_block_1x1_broken]spi 22 _rep 3 _w 1 _h 1}
sptcd]spi 102 _rep 1 _w 1 _h 1}
spr_turtle]spi 96 _rep 2 _w 1 _h 2}
spr_kani]spi 98 _rep 2 _w 2 _h 2}
spka]spi 106 _rep 2 _w 2 _h 2}
spkc]spi 118 _rep 2 _w 2 _h 1}
spegg]spi 4 _rep 1 _w 1 _h 1}
spci]spi 48 _rep 3 _w 1 _h 1}
spr_block_1x1]spi 16 _rep 1 _w 1 _h 1}
spr_concrete]spi 32 _rep 1 _w 1 _h 1}
spcg]spi 51 _rep 1 _w 1 _h 1}
spcbg]spi 9 _rep 1 _w 1 _h 1}
spcbr]spi 8 _rep 1 _w 1 _h 1}
spcic]spi 10 _rep 1 _w 1 _h 1}
spex]spi 33 _rep 3 _w 1 _h 1}
spgc]spi 83 _rep 8 _w 1 _h 1}
spci_empty]spi 34 _rep 1 _w 1 _h 1}
spfwll]spi 21 _rep 1 _w 1 _h 1}
spba]spi 22 _rep 1 _w 1 _h 1}
spbb]spi 23 _rep 1 _w 1 _h 1}
spbc]spi 24 _rep 1 _w 1 _h 1}
spgd]spi 17 _rep 1 _w 1 _h 1}
spbel]spi 6 _rep 1 _w 1 _h 1}
spber]spi 7 _rep 1 _w 1 _h 1}
spr_pipe_top_0]spi 25 _rep 1 _w 1 _h 1}
spr_pipe_top_1]spi 26 _rep 1 _w 1 _h 1}
spr_pipe_stem_0]spi 81 _rep 1 _w 1 _h 1}
spr_pipe_stem_1]spi 82 _rep 1 _w 1 _h 1}
]])
mget2spr = {
[32]=ee.spr_concrete,
[33]=ee.spex,
[16]=ee.spr_block_1x1,
[51]=ee.spcg,
[9]=ee.spcbg,
[8]=ee.spcbr,
[10]=ee.spcic,
[22]=ee.spba,
[23]=ee.spbb,
[24]=ee.spbc,
[17]=ee.spgd
}
spridx_pipe=25
map2bgtype =
{
[16]=bgbk,
[32]=bgcc,
[51]=bgcc,
[ 9]=bgcc,
[ 8]=bgcc,
[10]=bgcc,
[ spridx_pipe ]=bgcc,
[33]=bgex,
[83]=bgco,
[103]=bgsl,
[102]=bgtt,
[98]=bgkn,
[21]=bgfw,
[4]=bgtype_egg,
[22]=bgba,
[23]=bgba,
[24]=bgba,
[17]=bgdr,
}
b2e =
{
[bgsl]=ee.ets,
[bgtt]=ee.ett,
[bgkn]=ee.etk,
[bgtype_egg]=ee.ete,
}
enemytype2spr =
{
[ee.ets]=ee.spsl,
[ee.ett]=ee.spr_turtle,
[ee.etk]=ee.spr_kani,
[ee.ete]=ee.spegg,
}
enemytype2spr_crushed =
{
[ee.ets]=ee.spsl_crushed,
[ee.ett]=ee.sptcd,
[ee.etk]=ee.spkc,
}
bgtype2sensor_register =
{
[bgcc]=srw,
[bgbk]=srw,
[bgco]=sro,
[bgex]=srw,
[bgdr]=srd,
}
map_bgtype2spr = {
[bgcc]=ee.spr_concrete,
[bgbk]=ee.spr_block_1x1,
[bgco]=ee.spci,
[bgex]=ee.spex,
[bgdr]=ee.spgd,
}
map_bgtype2prio = {
[bgcc]=ee.prmp,
[bgbk]=ee.prmp,
[bgco]=ee.prco,
[bgex]=ee.prmp,
[bgba]=ee.prbg,
[bgdr]=ee.prco, 
}
function get_label(xx,yy)
local label = 0
for nn=0,3 do
local mm=mget(xx,yy-nn)
if mm>=spridx_0 and mm<=spridx_0+9 then
label*=10
label+=(mm - spridx_0)
else
return  label
end
end
return  label
end
function parse_map()
_map_register={}
for yy=0,63 do
for xx=0,127 do
if mget(xx,yy)==spridx_s then
local label=get_label(xx,yy-1)
local ex,ey
for rx=xx+1,127 do
for ry=yy,0,-1 do
local mm=mget(rx,ry)
if mm==spridx_e then
local e_label=get_label(rx,ry-1)
if label==e_label then
ex=rx-1
ey=ry
end
end
end
if ex!=nil then break end
end
_map_register[label]={
_label=label,
_sx=xx+1,
_sy=yy,
_ex=ex,
_ey=ey
}
end
end
end
end
parse_map()
_scrolls={}
str2table(_scrolls,
[[
scroll_title="s7 s444 s444 s99 p1 s100 g"
scroll_0="n16 y14 @9 y15 @8 >17 n12 y14 @9 y15 @8 n1 y10 @33 >13 |11 >3 s100 s99 g"
scroll_1="s8 s38 s39 s39 s50 s49 s49 s49 s50 p1 s100 s99 g"
scroll_2="s20 s21 s22 e0 s27 s27 s27 s27 s1 s1 s1 s1 s1 s1 s1 s1 s1 p1 s100 s99 g"
scroll_3="b1 s810 s8 b0 s1 s0 s1 s20 s21 s22 s25 s27 s27 s27 s27 s28 s28 s28 s1 s1 s28 s28 s29 s100 s99 g"
]]
)
function scroll_set( scroll_ )
_scroll=str2table(
{_str_scroll=scroll_},
[[
_sp=1
_look_back=false
_enemy_replace=1
_pole=0
_nn=0
_yy=12
]]
)
end
function sstg(stg_)
scroll_set(_scrolls["scroll_"..stg_])
end
function coin_inc( inc_ )
local inc=inc_ or 1
_income.coin+=inc
if _income.coin==100 then
_income.coin=0
_income.left+=1
end
end
function scg()
return sub(_scroll._str_scroll,_scroll._sp,_scroll._sp)
end
function scn()
local strnum=""
while scg()!=" " do
strnum=strnum..scg()
_scroll._sp+=1
end
_scroll._sp+=1
return tonum( strnum )
end
function scw()
local chr=scg()
if chr=="g" then
return nil
end
_scroll._sp+=1
if chr=="s" then
local _map=_map_register[ scn() ]
return _map
elseif chr=="b" then
_scroll._look_back=scn()
return nil
elseif chr=="e" then
_scroll._enemy_replace=scn()
return nil
elseif chr=="p" then
_scroll._pole=scn()
return nil
elseif chr==">" then
game.x_generated_scroll += 8*scn()
return scw()
elseif chr=="n" then
_scroll._nn=scn()
return scw()
elseif chr=="y" then
_scroll._yy=scn()
return scw()
elseif chr=="|" then
local bgtype=map2bgtype[spridx_pipe]
local nx = game.x_generated_scroll+4
local ytop = scn()*8+4
for yyy=ytop,128,8 do
for nn=0,1 do
local bgobj=new_bgobj(bgtype)
bgobj.x,bgobj.y=nx+(nn*8),yyy
if yyy==ytop then
bgobj.asp.idx_label=ee.spr_pipe_top_0+nn
else
bgobj.asp.idx_label=ee.spr_pipe_stem_0+nn
end
end
end
return scw()
elseif chr=="@" then
local spridx=scn()
local bgtype=map2bgtype[spridx]
for nn=0,_scroll._nn-1 do
local bgobj=new_bgobj(bgtype)
bgobj.x,bgobj.y=game.x_generated_scroll+nn*8+4,_scroll._yy*8+4
bgobj.asp.idx_label=mget2spr[spridx]
if bgtype==bgex then
bgobj.insentive=ee.ins_coin
end
end
return scw()
end
end
sstg(0)
function merge( lhs_ , rhs_ )
for k,v in pairs( rhs_ ) do
lhs_[k]=v
end
return lhs_
end
_reset_req,_crr=0,0
function	_reset( reset_req_ )
_crr=reset_req_
if reset_req_==ee.rrt then
scroll_set(_scrolls.scroll_title)
else
sstg(_income.stg)
end
scot()
_ar()
sres()
game=str2table(
{
camera={x=0,y=0},
penpen_head={x=0,y=0}
},
[[
frqh=0
over=0
frm=0
frm_update60=0
rqh=0
frm_pause=0
x_generated_scroll=0
cnt_generated_scroll=0
cnt_ice_pole_brk=0
req_kill_clone=false
]]
)
if _crr==ee.rrc then
scco(0)
else
scco(12)
new_penpen(false)
end
end
function reset_req(reset_req_)
_reset_req=reset_req_
end
function reset_recv()
if _reset_req > 0 then
local req=_reset_req
_reset_req=0
_reset(req)
end
end
function income_reset()
_income={left=3,coin=0,stg=0}
end
function game_pause( frm_pause_ )
game.frm_pause=frm_pause_
end
function tett(th)
tx,ty,tvx,tvy=th.x,th.y,th.vx,th.vy
end
function teet(th)
th.x,th.y,th.vx,th.vy=tx,ty,tvx,tvy
end
function _update60()
	game.frm_update60+=1
if game.frm_pause>0 then
game.frm_pause-=1
end
if game.frm_pause==0 then
cup(game.camera)
btn_update60()
ssrc()
for actor in all(actors) do
if actor.reqkill == false and actor.on_update != nil then
tett(actor)
actor.on_update(actor)
teet(actor)
end
end
sw.enb_touch_sensor = true
for actor in all(actors) do
if actor.reqkill == false and actor.on_touch != nil then
tett(actor)
actor.on_touch(actor)
teet(actor)
end
end
for actor in all(actors_req_kill) do
tett(actor)
dact(actor)
teet(actor)
del(actors,actor)
end
actors_req_kill={}
end
outgame_update();
reset_recv()
end
function scco(col)
_col=col
end
scco(12)
function _draw()
rectfill(0,0,127,127,_col)
camera(0,0)
scot()
for actor in all(actors) do
if actor.reqkill==false and actor.on_draw!=nil then
tett(actor)
actor.on_draw(actor)
teet(actor)
end
end
sdot()
outgame_draw()
_print_draw()
end
function btn_initialize()
btnwork={tp={}}
for ii=0,5 do
btnwork.tp[ii] = 0
end
end
btn_initialize()
function btn_update60()
for ii=0,5 do
if btn(ii) then
btnwork.tp[ii]+=1
else
btnwork.tp[ii]=0
end
end
end
function btn_tp(idx)
return btnwork.tp[idx]
end
function _print_reset()
pw={}
pw.print_actors={}
end
_print_reset()
function _print(str_,col_,timeout_)
	if timeout_==nil	then timeout_=10	end
	if col_== nil then col_=7		end
	add(pw.print_actors,{str=str_,col=col_,timeout=timeout_})
end
function pas(astr)
	astr.timeout=astr.timeout-1
	if pw.yy>=128 then
		return
	end
	color(astr.str)
	print(astr.str,0,pw.yy,astr.col)
	pw.yy=pw.yy+6
end
function	_print_draw()
	pw.yy=8
	foreach(pw.print_actors,pas)
	for astr in all(pw.print_actors) do
		if astr.timeout<=0 then del(pw.print_actors,astr) end
	end
end
function tcp(orig)
local orig_type,copy=type(orig)
if orig_type=='table' then
copy={}
for orig_key,orig_value in pairs(orig) do
copy[orig_key]=orig_value
end
else
copy=orig
end
return copy
end
function new_array()
return {
_buff={},
_wrap=awa,
_size=0,
_idx=0
}
end
function	acl(th)
th._size,th._idx,th._buff=0,0,{}
end
function	apb(th,xx_)
th._buff[th._size]=tcp(xx_)
th._size=th._size+ 1
end
function	asw(th,wrap_)
th._wrap=wrap_
end
function	asz(th)
return	th._size
end
function	ast(th,idx_,v_)
	if th._size==0 then
		return
	end
	if th._wrap==awa then
		th._buff[idx_]=tcp(v_)
	elseif  th._wrap==awr then
		th._buff[idx_%th._size]=tcp(v_)
	else
	end
end
function	arf(th,idx_)
	if th._wrap==awa then
		return	th._buff[idx_]
	elseif  th._wrap==awr then
		local midx=idx_%th._size
		return	th._buff[midx]
	else
	end
end
function	_clamp(x_,min_,max_)
return  min(max(x_,min_),max_)
end
function	cup(cam)
cam.x=max(cam.x,game.penpen_head.x-50)
	cam.y=0;
end
function	new_asp_ex()
return str2table(
{
camera=game.camera,
pal={}
},
[[
idx_label=0
x=0
y=0
offset_x=0
offset_y=0
fx=false
fy=false
frm_anim=0
priority=0
]]
)
end
function asp_ex_set(th,idx_label_)
th.idx_label=idx_label_
local sp=gg.sprites_ex[th.idx_label]
th.offset_x,
th.offset_y
=
-(sp._w*8)/2,
-4-(sp._h-1)*8
end
function	scot()
spmx,sot=8,{}
end
function	sdot()
for prio=0,spmx-1 do
if sot[prio]!=nil then
for ii=0,asz(sot[prio])-1 do
local arg_=arf(sot[prio],ii)
local sp=gg.sprites_ex[arg_.idx_label]
for key,val in pairs(arg_.pal) do
pal(key,val)
end
spr(
sp.spi+(flr(arg_.frm_anim)%sp._rep)*sp._w,
arg_.lx,arg_.ly,
sp._w,sp._h,
arg_.fx,arg_.fy
)
for key,val in pairs(arg_.pal) do
pal(key,key)
end
end
end
end
scot()
end
function	spx2(arg_)
local sp,lx,ly
=
gg.sprites_ex[arg_.idx_label],
arg_.x+arg_.offset_x-arg_.camera.x,
arg_.y+arg_.offset_y-arg_.camera.y
if lx+sp._w*8<0 then return end
if lx>=128 then return end
if ly+sp._h*8<0 then return end
if ly>=128 then return end
arg_.lx,arg_.ly=lx,ly
if sot[arg_.priority]==nil then
sot[arg_.priority]=new_array()
end
apb(sot[arg_.priority],arg_)
end
function	_rnd(p0,p1)
	return	rnd(p1-p0)+p0
end
function	nact()
local th=str2table({},
[[
x=0
y=0
vx=0
vy=0
reqkill=false
vflip=false
hflip=false
]]
)
	add(actors,th)
	return th
end
function _ar()
	actors={}
	actors_req_kill={}
end
function	dact(th)
	if th.on_delete!=nil then
		th.on_delete(th)
	end
end
function	ape(th,ev_)
	th.ev_que=th.ev_que or new_array()
apb(th.ev_que,ev_)
end
function	acne(th,idx_)
if th.ev_que then
return  asz(th.ev_que)
else
return 0
end
end
function	actor_ref_ev(th,idx_)
if th.ev_que then
if idx_<asz(th.ev_que) then
return arf(th.ev_que,idx_)
end
end
return nil
end
function	ace(th)
if th.ev_que!=nil then
acl( th.ev_que )
end
end
function	akl(th)
if th.reqkill==true then return end
	if th.on_req_kill!=nil then
		th.on_req_kill(th)
	end
th.reqkill=true
add(actors_req_kill,th)
end
function acc(th)
local co = false
if tx<game.camera.x-64 then
co=true
elseif tx>game.camera.x+640 then
co=true
elseif ty>144 then
co=true
elseif ty<-96 then
co=true
end
if co==true then
akl(th)
end
return co
end
function	outgame_update()
	game.frm=game.frm+1
	if game.rqh>-100 then
		game.rqh=game.rqh-1
end
if _crr==ee.rrt then
	 if game.frm>300 then
reset_req(ee.rrt)
return
elseif btn_tp(4)==1 or btn_tp(5)==1 then
income_reset()
reset_req(ee.rrc)
return
end
elseif _crr == ee.rrc then
	 if game.frm>120 then
if _income.left==0 then
reset_req(ee.rrt)
else
reset_req(ee.rrps)
end
end
end
if _crr!=ee.rrc then
outgame_genbg()
end
end
function	outgame_draw()
color(7)
if _crr==ee.rrc then
if _income.left>0 then
local yy=56
spr(14,32,yy,2,2,true)
print("x ".._income.left,64,66)
else
print("game over",48,64)
end
end
if _crr!=ee.rrt then
spr(83,104,0,1,1,true)
print("x".. _income.coin,112,3)
spr(80,0,0,1,1)
print("x".. _income.left,9,3)
print("w".. flrshr(_income.stg,2)+1 .. "-" .. (_income.stg%4)+1,56,3)
end
end
function	outgame_genbg()
while game.x_generated_scroll<game.camera.x+192 do
local map=scw()
if nil==map then return end
for yy=map._sy,map._ey,-1 do
for xx=map._sx,map._ex,1 do
local spridx=mget(xx,yy)
local bgtype=map2bgtype[spridx]
local lx,ly=xx-map._sx,15-(map._sy-yy)
if bgtype!=nil then
local nx,ny=game.x_generated_scroll+lx*8+4,ly*8+4
if
bgtype==bgcc or
bgtype==bgbk or
bgtype==bgco or
bgtype==bgex or
bgtype==bgba or
bgtype==bgdr
then
local bgobj=new_bgobj(bgtype)
bgobj.x,bgobj.y=nx,ny
if _scroll._pole>0 then
local spr=mget2spr[spridx]
if spr==ee.spcic then
bgobj._pole=_scroll._pole
end
end
if bgtype==bgex then
bgobj.insentive=ee.ins_coin
end
if bgtype==bgcc or bgtype==bgba then
bgobj.asp.idx_label=mget2spr[spridx]
end
elseif bgtype==bgfw then
local fw=new_floating_wall()
fw.x,fw.y=nx,ny
elseif b2e[bgtype]!=nil then
if _scroll._enemy_replace>0 then
local en=new_enemy(b2e[bgtype])
en.x,en.y,en._look_back=nx,ny,_scroll._look_back
if b2e[bgtype]==ee.ett then
if en._look_back==1 then
en.asp.pal[11]=8
end
end
end
end
end
end
end
game.x_generated_scroll+=(map._ex-map._sx+1)*8
end
end
function new_effect()
local th=nact()
merge(th,
{
on_update=effect_update,
on_draw=effect_draw,
cnt_update=0,
frm_update=0,
asp=new_asp_ex()
}
)
th.asp.idx_label,
th.asp.priority
=
ee.spr_block_1x1_broken,
ee.pbbk
return  th
end
function new_effect_piece( piece_type_ )
local eff = new_effect()
if piece_type_ == ee.pti then
eff.asp.pal = {
[4]=12, [9]=7, [15]=7
}
end
return  eff
end
function	effect_update(th)
th.cnt_update += 1
if tvx > 0 then
th.frm_update += 1
else
th.frm_update -= 1
end
tvy += 0.21
tx += tvx
ty += tvy
if
tx < game.camera.x - 16     or
tx > game.camera.x + 144    or
ty > 136 or
ty < -128
then
akl(th)
end
end
function	effect_draw(th)
th.asp.x,th.asp.y = tx-4,ty-4
if th.asp.idx_label == ee.spgc then
th.asp.frm_anim = flr( ( th.cnt_update-1 )/3 )
if th.asp.frm_anim > 7 then
return
end
else
local key = arf( anb, flrshr( th.frm_update , 2 ) )
local as = th.asp
as.fx,
as.fy,
as.frm_anim
=
key.xf,
key.yf,
key.idx
end
spx2( th.asp )
end
_nbgobj = str2table(
{},
[[
hp=1
frm_draw=0
shake_y=0
ang_shake=0
frm_coin_bonus=100
_pole=false
]]
)
function	new_bgobj(bgtype_)
	local th = nact()
if bgtype_ == bgco then
th.on_draw = bgobj_draw_coin
elseif bgtype_ == bgex then
th.on_draw = bgobj_draw_excl
else
th.on_draw = bgobj_draw
end
merge(th,_nbgobj)
th._bgtype,
th._walltype,
th.on_update,
th.on_req_kill,
th.on_touch,
th.lpos_hitstop,
th.insentive,
th.sts_coin
=
bgtype_,
ee.walltype_a,
bgobj_update,
bgrk,
bgth,
{0,0},
ee.ins_none,
ee.coin_idle
th.asp = new_asp_ex()
th.asp.idx_label,
th.asp.priority
=
map_bgtype2spr[ bgtype_ ],
map_bgtype2prio[ bgtype_ ]
return	th
end
function bgns(th)
th.sensor_info = nsi()
return th.sensor_info
end
function bgsu(th)
if th.sensor_info != nil then
suab( th.sensor_info )
th.sensor_info = nil
end
end
function bgrk(th)
bgsu(th)
end
function	bgcu(th)
tx += tvx
ty += tvy
tvy *= 0.8
if abs( tvy ) < 0.1 then
akl(th)
end
end
function	bgth(th)
if th._bgtype == bgba then
return
end
local num_ev = acne(th)
for idx=0,num_ev-1 do
local ev = actor_ref_ev(th,idx)
if ev == nil then break end
local hit,ice_brk = false,false
if th._bgtype == bgbk then
if
ev.evid == ee.edpj or
ev.evid == ee.edsp
then
hit = true
end
elseif th._bgtype == bgex then
if
ev.evid == ee.edpa or
ev.evid == ee.edsp
then
if th.sts_coin == ee.coin_idle then
hit = true
th.sts_coin = ee.coin_bonus
elseif th.sts_coin == ee.coin_bonus then
hit = true
if th.frm_coin_bonus < 0 then
th.sts_coin,
th.asp.idx_label
=
ee.coin_empty,
ee.spci_empty
end
end
end
end
if th._pole == 1 and ev.evid == ee.edip then
hit = true
ice_brk = true
game.cnt_ice_pole_brk += 1
end
if hit == true then
if th.insentive == ee.ins_coin then
local cf = new_effect()
coin_inc()
if ev.evid == ee.edsp then
local vx = 1.1
if ev.xpunch > tx then
vx = -vx
end
cf.x,
cf.y,
cf.vx,
cf.vy,
cf.asp.idx_label
=
tx + vx,
ty,
vx,
-1.7,
ee.spgc
else
cf.x,
cf.y,
cf.vx,
cf.vy,
cf.asp.idx_label
=
tx,
ty - 8,
0,
-3.2,
ee.spgc
end
end
stc(
tx-3,
ty-3-8,
6,
6,
sro,
function(sensor)
ape(
sensor._actor ,
{
evid = ee.edgc,
bounce = true,
xpunch = ev.xpunch
}
)
end
)
stc(
tx-6,ty-8,
12,4,
sre,
function(sensor)
ape(
sensor._actor ,
{
evid = ee.edbs,
bounce = true,
xpunch = ev.xpunch
}
)
end
)
local voff = 0.0
if ev.evid == ee.edsp then
voff = 1.2
if tx < ev.xpunch then
voff = -voff
end
end
if ice_brk == true then
voff = _rnd(0.7,1.7)
end
if th._bgtype == bgbk or ice_brk == true then
local _vx, _vy0,_vy1 = 0.7,-3.5,-2.0
local pt = ee.ptb
if ice_brk == true then
pt = ee.pti
end
for ii=0,3 do
local broken,dir =
new_effect_piece(pt),
0.125 + ii*0.25
broken.vx, broken.vy =
cos(dir) + voff,
sin(dir) - 3.5
broken.x, broken.y =
tx+broken.vx,ty+broken.vy
end
akl(th)
return
elseif th._bgtype == bgex then
th.ang_shake = 0.5
do break end
end
elseif
ev.evid == ee.edgc and
th._bgtype == bgco
then
coin_inc()
if ev.bounce == true then
tvy,
th.on_update,
th.on_touch
=
-7,
bgcu,
nil
bgsu(th)
else
local cf = new_effect()
cf.x,
cf.y,
cf.vx,
cf.vy,
cf.asp.idx_label
=
tx,
ty,
0,
-3.5,
ee.spgc
bgsu(th)
akl(th)
end
return
end
end
ace(th)
end
function	bgobj_update(th)
if tx < game.camera.x - 8*8 then
akl(th)
return
end
if th._bgtype == bgba then
return
end
if th.sts_coin == ee.coin_bonus then
th.frm_coin_bonus -= 1
end
if th.sensor_info == nil then
local info = bgns(th)
info.x,
info.y,
info.w,
info.h,
info._actor,
info._register
=
tx-4,
ty-4,
8,
8,
th,
bgtype2sensor_register[ th._bgtype ]
srab(info)
end
if th.ang_shake > 0 then
th.ang_shake = max( th.ang_shake - 0.04 , 0 )
th.shake_y = 8*sin( th.ang_shake )
else
th.ang_shake,th.shake_y = 0,0
end
end
function	bgobj_draw(th)
th.frm_draw,
th.asp.x,
th.asp.y
=
th.frm_draw +1,
tx-4,
ty-4 + th.shake_y
spx2( th.asp )
end
function	bgobj_draw_coin(th)
th.frm_draw += 1
th.asp.x,
th.asp.y,
th.asp.frm_anim
=
tx-4,
ty-4,
arf( anc , flrshr(game.frm_update60,3) )
spx2( th.asp )
end
function bgobj_draw_excl(th)
th.frm_draw += 1
th.asp.x,
th.asp.y
=
tx-4,
ty-4+th.shake_y
th.asp.frm_anim = arf(
ane,
flrshr(game.frm_update60,3)
)
spx2( th.asp )
end
function	pnas( spr_ ,is_clone_ )
local aa = new_asp_ex()
if is_clone_ == true then
aa.priority = ee.ppc
aa.pal[1]=14
else
aa.priority = ee.ppe
end
asp_ex_set( aa , spr_ )
return aa
end
function	new_penpen(is_clone_)
return merge(
nact(),
str2table(
{
on_update = penpen_update,
on_touch= penpen_touch,
on_draw= penpen_draw,
_is_clone=is_clone_,
_jmp_state = ee.jswj,
asp = pnas(ee.sppp,is_clone_ ),
asp_stop = pnas(ee.sppp_stop,is_clone_ ),
asp_jump = pnas(ee.sppp_jump,is_clone_ ),
asp_slip = pnas(ee.sppp_slip,is_clone_ ),
asp_kick = pnas(ee.sppp_kick,is_clone_ ),
},
[[
_cnt=0
_cnt_unrivaled=0
_cnt_pause=0
_cnt_kick=0
x=40
y=20
vx=0
vy=0
hflip=true
_is_slip=false
_cnt_jmp=0
_cnt_upper_collide=0
latest_anim=0
in_door=false
]]
)
)
end
function	pbt(th,idx)
if _crr == ee.rrt then
if idx == 1 then
if game.frm > 70 then
return 1
else
return 0
end
else
return 0
end
end
if th.in_door == true then
return 0
elseif th.chk_only_bottom == true then
if idx == 1 then
return 1
else
return 0
end
end
return  btnwork.tp[idx]
end
function	penpen_update(th)
local any_dir = false
th._is_slip = false
if th._cnt_kick > 0 then
th._cnt_kick -= 1
end
if th._is_clone == true then
if game.req_kill_clone == true then
pedc(th)
return
end
end
if ty > 160 then
if th._is_clone == true then
akl(th)
else
pedc(th)
end
return
end
local num_ev = acne(th)
for idx=0,num_ev-1 do
local ev = actor_ref_ev(th,idx)
if ev.evid == ee.edko_penpen then
pedc(th)
return
end
end
if th._jmp_state == ee.jstg then
tvy += grv
if pbt(th,5) == 1 then
tvy,
th._jmp_state,
th._cnt_jmp
=
pvk,
ee.jskj,
0
else
if pbt(th,0) > 0 then
tvx -= pag
th.hflip = false
any_dir = true
if tvx > 0.5 then
th._is_slip = true
end
elseif pbt(th,1) > 0 then
tvx += pag
th.hflip = true;
any_dir = true
if tvx < -0.5 then
th._is_slip = true
end
end
if any_dir == false then
tvx *= 0.73
end
end
th.latest_anim = flr( tx/3 )
elseif th._jmp_state == ee.jskj then
th._cnt_jmp += 1
tvy = pvk
if pbt(th,0) > 0 then
tvx -= paa
elseif pbt(th,1) > 0 then
tvx += paa
end
if th._cnt_upper_collide > 0 then
tvy = - tvy
th._jmp_state = ee.jswj
elseif pbt(th,5) == 0 then
tvy = 0
th._jmp_state = ee.jswj
elseif pbt(th,5) > 11 then
tvy += grv
th._jmp_state = ee.jswj
end
elseif  th._jmp_state == ee.jswj      then
if pbt(th,0) > 0 then
tvx -= paa
elseif pbt(th,1) > 0 then
tvx += paa
end
tvy += grv
else
end
th._cnt_upper_collide,
tvx,
tvy
=
0,
_clamp( tvx , -1.5 , 1.5 ),
_clamp( tvy , -3.5 , 3.5 )
if th._cnt_pause == 0 then
tx += tvx;
ty += tvy;
end
if th._cnt_pause > 0 then
th._cnt_pause -= 1
end
tx = max( tx , game.camera.x+2 )
if th._is_clone == true then
if tx < game.camera.x - 16 or tx > game.camera.x + 144 then
akl(th)
return
end
else
game.penpen_head.x,
game.penpen_head.y
= tx,ty
end
local info = nsi()
merge(info,
{
x = tx,
y = ty-4,
w = 3,
h = 3,
_actor = th,
_register = srpb
}
)
sad(info)
end
function ptfw(th)
stc(
tx,ty+4,1,1,srfw,
function(sensor)
tx += sensor.vx
ty += sensor.vy
end
)
end
function	penpen_touch_wall(th)
local wall = srw
if tvy >= 0 then
stc(
tx-2,ty+4,4,2,wall,
function(sensor)
ty,
tvy,
th._jmp_state
=
min( ty, sensor.y-4),
0,
ee.jstg
end
)
end
if th.chk_only_bottom then
return
end
stc(
tx+3,ty,1,1,wall,
function(sensor)
tx,tvx = min( tx, sensor.x-4),0
if sensor._actor._pole == 1 and th._is_clone == false then
penpen_do_pole(th)
ape(
sensor._actor,
{ evid = ee.edip, }
)
return true
end
end
)
stc(
tx-4,ty,1,1,wall,
function(sensor)
tx,tvx = max(tx,sensor.x+sensor.w+4),0
end
)
local maxdis = 128
local nearest_sensor = nil
stc(
tx-2,ty-4,4,2,wall,
function(sensor)
local cx = (sensor.x+sensor.w*0.5)
local distance = abs( cx - tx )
if distance < maxdis then
nearest_sensor = sensor
maxdis = distance
ape(
sensor._actor ,
{
evid = ee.edpa,
xpunch = tx
}
)
end
end
)
if nearest_sensor then
ty,tvy = max( ty, nearest_sensor.y + nearest_sensor.h+4),0
th._cnt_upper_collide += 1
ape(
nearest_sensor._actor ,
{
evid = ee.edpj,
xpunch = tx
}
)
end
end
function atam(th,sensor_register_,func_)
stc( tx-3,ty-3,6,6,sensor_register_,func_)
end
function pedc(th)
if th._is_clone == false then
game.req_kill_clone=true
end
th.on_update,
th.on_draw,
th.on_touch,
th.cnt_penpen_out,
tvy
=
penpen_update_crushed,
penpen_draw_crushed,
nil,
0,
-3.2
end
function pete(th)
if th._cnt_unrivaled > 0 then
th._cnt_unrivaled -= 1
end
stc(
tx-4,ty-4,8,8,
bor(sre,srec),
function(sensor)
if band( sensor._register , srec ) == 0 then
if th._cnt_unrivaled == 0 then
pedc(th)
end
else
pedk(th,sensor._actor )
end
end
)
stc(
tx-4,ty-4,8,8,
sreg,
function(sensor)
pedk(th,sensor._actor)
end
)
end
function pedk(th,actor)
str2table(th,
[[
_cnt_unrivaled=10
_cnt_pause=3
_cnt_kick=9
]]
)
ape(
actor,
{
evid = ee.edpk,
actor_kicker = th,
xpunch = tx
}
)
end
function pets(th)
if tvy < 0 then
return  0
end
local cnt_step_on = 0
stc(
tx-6,ty+5,12,2,
bor( sre, srec),
function(sensor)
if band( sensor._register , srec ) > 0 then
pedk(th,sensor._actor)
return true
else
ape(
sensor._actor ,
{ evid = ee.edso, bounce = true, _actor = th, xpunch = tx }
)
cnt_step_on += 1
end
end
)
if cnt_step_on > 0 then
tvy = -2.4*cnt_step_on
ty += tvy
end
return  cnt_step_on
end
function	cbu(th)
th.cnt_update += 1
if band(th.cnt_update,31) == 0 then
if th.cnt_fired >= game.cnt_ice_pole_brk then
th.cnt_update = 0
th.on_update = cbu_clear
return
end
local xx,yy,_vx,_vy0,_vy1 =
_rnd( game.camera.x + 32 , game.camera.x + 96 ),_rnd( 32,70 ),0.7,-3.5,-2.0
coin_inc(3)
for ii=0,7 do
local broken,dir =
new_effect_piece(ee.pti),
0.125 + ii*0.125
broken.vx, broken.vy =
cos(dir),
sin(dir) - 3.5
broken.x, broken.y =
xx+broken.vx,yy+broken.vy
end
th.cnt_fired += 1
end
end
function cbu_clear(th)
th.cnt_update += 1
if th.cnt_update == 60 then
_income.stg+=1
reset_req( ee.rrc )
end
end
function petp(th)
stc(
tx-3,
ty-3,
6,6,
srpb,
function(sensor)
if sensor._actor != th then
if sensor._actor.x < tx then
tx+=1
elseif tx < sensor._actor.x then
tx-=1
end
end
return false
end
)
end
function	penpen_touch(th)
ptfw(th)
penpen_touch_wall(th)
petp(th)
atam(
th,sro,
function(sensor)
ape(
sensor._actor,
{
evid = ee.edgc,
bounce = false
}
)
end
)
if th.chk_only_bottom == true and th.in_door == false then
atam(
th,srd,
function(sensor)
th.on_draw,th.in_door = nil,true
local cb = nact()
cb.cnt_update,
cb.cnt_fired,
cb.on_update =
0,0,cbu
end
)
end
if 0 == pets(th) then
pete(th)
end
end
function	penpen_draw(th)
local arg = th.asp
local hflip = th.hflip
if abs( tvx ) < 0.3 then
arg = th.asp_stop
end
if th._jmp_state == ee.jskj or
th._jmp_state == ee.jswj then
if tvy < 0 then
arg = th.asp_jump
end
else
if th._is_slip == true then
arg = th.asp_slip
if hflip == true then
hflip = false
else
hflip = true
end
end
end
if th._cnt_kick > 0 then
arg = th.asp_kick
end
merge(arg,{
x = tx,
y = ty,
fx = hflip,
fy = th.vflip,
frm_anim = th.latest_anim,
})
spx2(arg)
end
function	penpen_update_crushed(th)
local delay=0
if th._is_clone == true then
delay=17
end
th.cnt_penpen_out += 1
if th.cnt_penpen_out == 1 and th._is_clone == false then
game_pause(35)
end
if th.cnt_penpen_out >= 2+delay then
tvy += grv
ty += tvy
end
if th.cnt_penpen_out >= 130+delay then
if th._is_clone == false then
_income.left -= 1
reset_req( ee.rrc )
end
akl(th)
end
end
function penpen_draw_crushed(th)
if ty < 128+8 then
local arg = th.asp
merge(arg,{
x = tx,
y = ty,
fx = th.hflip,
fy = true
})
spx2(arg)
end
end
function penpen_do_pole(th)
th.on_update,
th.on_draw,
th.on_touch,
th._cnt_pause
=
penpen_update_pole,
penpen_draw_pole,
penpen_touch_pole,
40
end
function	penpen_update_pole(th)
if th._cnt_pause == 0 then
ty += 3
end
if th._cnt_pause > 0 then
th._cnt_pause = max(th._cnt_pause-1,0)
end
end
function	penpen_touch_pole(th)
stc(
tx+5,ty-5,1,3,srw,
function(sensor)
if sensor._actor._pole == 1 then
tx = min( tx, sensor.x-4)
th._cnt_pause += _rnd(1,4)
th.latest_anim += 1
ape(
sensor._actor,
{ evid = ee.edip, }
)
return true
end
end
)
stc(
tx-2,ty+4,4,2,srw,
function(sensor)
ty,
tvy
=
min( ty, sensor.y-4),
0
th.on_update,
th.on_touch,
th.on_draw,
th._jmp_state,
th._cnt_pause,
th.chk_only_bottom
=
penpen_update,
penpen_touch,
penpen_draw,
ee.jswj,
0,
true
return true
end
)
end
function	penpen_draw_pole(th)
local arg = th.asp
merge(arg,{
x = tx,
y = ty,
fx = true,
fy = false,
frm_anim = th.latest_anim
})
spx2(arg)
end
function new_floating_wall()
local th = nact()
merge(th,
{
on_update  = floating_wall_update,
on_draw    = floating_wall_draw,
asp = new_asp_ex(),
}
)
str2table(th,
[[
cnt_update=0
ix=0
iy=0
nw=5
]]
)
asp_ex_set( th.asp ,ee.spfwll )
th.asp.priority = ee.prfw
return th
end
function floating_wall_update(th)
if acc(th) == true then
return
end
th.cnt_update += 1
if th.cnt_update == 1 then
th.ix = tx
th.iy = ty
end
local info,ox,oy = nsi(),tx,ty
tx = th.ix + 18*sin( th.cnt_update / 300 )
info.vx,info.vy = tx - ox,ty - oy
info.w,info.h = 8 * th.nw,8
info.x,info.y = tx - info.w/2,ty-4
info._actor,
info._register
=
th,
bor(srfw,srw)
sad(info)
end
function floating_wall_draw(th)
local arg = th.asp
arg.offset_x = -(th.nw*8)/2
for nn=0,th.nw-1 do
arg.x,arg.y = tx,ty
spx2( arg )
arg.offset_x += 8
end
end
function new_enemy(enemytype_)
local th = nact()
local lvx
if enemytype_ == ee.etk then
lvx = - espe_kani
elseif enemytype_ == ee.ete then
lvx = 0
else
lvx = - espe
end
merge(th,{
etp = enemytype_,
vx = lvx,
on_update  = eneuwfa,
asp = new_asp_ex(),
})
str2table( th,
[[
isl=false
is_crushed=false
cnt_crushed=0
fpp=0
is_on_ground=false
cnt_not_crushed=0
anger=0
_look_back=0
]]
)
asp_ex_set( th.asp , enemytype2spr[ th.etp ] )
th.asp.priority = ee.pren
return th
end
function ensp(th)
if th.fpp == 0 then
tvy += grv
tvx = _clamp( tvx , -3 , 3 )
tvy = _clamp( tvy , -3.5 , 3.5 )
tx += tvx
ty += tvy
end
if th.fpp > 0 then
th.fpp -= 1
end
end
function eneuwfa(th)
if acc(th) == true then
return
end
if tx < game.camera.x + 128 + 32 then
th.on_update,
th.on_touch,
th.on_draw
=
enud,
entouch,
enemy_draw
end
end
function enacm(th,register_)
local info = nsi()
merge(info,
{
x = tx-2,
y = ty+1,
w = 4,
h = 3,
_actor = th,
_register = register_
}
)
sad(info)
end
function endko(th,ev)
th.on_update,
th.on_draw,
th.on_touch,
tvx,
tvy
=
eko,
enedk,
nil,
0.13*(tx - ev.xpunch),
-2
end
function endcr(th,ev)
merge(th,
{
cnt_crushed = 0,
on_update  = enud_crushed,
on_draw    = edcsed,
cnt_not_crushed = 7,
is_crushed = true
}
)
asp_ex_set( th.asp , enemytype2spr_crushed[ th.etp  ] )
end
function enud(th)
if acc(th) == true then
return
end
local num_ev = acne(th)
for idx=0,num_ev-1 do
local ev = actor_ref_ev(th,idx)
if ev == nil then break end
if ev.evid == ee.edso then
if th.etp == ee.etk then
ape(
ev._actor,
{ evid = ee.edko_penpen }
)
else
if th.cnt_not_crushed == 0 then
endcr(th,ev)
ace(th)
return
end
end
elseif ev.evid == ee.edpk and th.etp == ee.ete then
for ii=0,1 do
local broken = new_effect_piece(ee.pti)
broken.x,broken.y,broken.vy = tx,ty,-2.3
broken.vx = 2*(ii-0.5)
if ii == 0 then
broken.asp.idx_label = ee.spbel
else
broken.asp.idx_label = ee.spber
end
end
local pen = new_penpen(true)
pen.x,pen.y = tx,ty
akl(th)
elseif ev.evid == ee.edko then
endko(th,ev)
ace(th)
return
elseif
ev.evid == ee.edbs
then
if th.etp == ee.ett then
endcr(th,ev)
edb(th,ev.xpunch)
ace(th)
return
elseif th.etp == ee.etk then
if th.anger == 1 then
th.anger = 0
endcr(th,ev)
else
th.asp.idx_label = ee.spka
th.anger = 1
end
edb(th,ev.xpunch)
local lvx = (th.anger+1)*espe_kani
if tvx < 0 then
tvx = -lvx
else
tvx =  lvx
end
ace(th)
return
else
endko(th,ev)
ace(th)
return
end
end
end
ace(th)
if th.cnt_not_crushed > 0 then
th.cnt_not_crushed -= 1
end
ensp(th)
if th.etp == ee.ete then
enacm(th, sreg )
else
enacm(th, sre )
end
end
function entbw(th,sensor)
if sensor._actor != th then
ty = min( ty, sensor.y-4)
tvy,th.is_on_ground = 0,true
end
end
function entouch(th)
if tvy >= 0 then
th.is_on_ground = false
if th.isl == false then
stc(
tx-1,ty+4, 2,2, srw,
function(sensor)
entbw(th,sensor)
end
)
else
local xoff = 1
if tvx < 0 then xoff = -1 end
stc(
tx+xoff,ty+4, 1,2, srw,
function(sensor)
entbw(th,sensor)
end
)
end
end
local any_collide = false
local actor_any_collide = nil
if tvx < 0 then
stc(
tx-4,ty,
1,1,
srw,
function( sensor )
tx,
tvx,
actor_any_collide,
any_collide
=
max(tx, sensor.x+sensor.w+4),
- tvx,
sensor._actor,
true
return true
end
)
else
stc(
tx+3,ty,
1,1,
srw,
function( sensor )
tx,
tvx,
actor_any_collide,
any_collide
=
min( tx, sensor.x-4),
- tvx,
sensor._actor,
true
return true
end
)
end
if th.is_crushed == false and th.etp == ee.ett and th._look_back == 1 and th.fpp == 0 and th.isl == false and tvy>=0 then
local xoff = 4
if tvx < 0 then
xoff = -xoff
end
if asz( stch(
tx+xoff,ty+4,
1,1,
srw
)) == 0 then
tvx = - tvx
any_collide = true
end
end
if actor_any_collide != nil then
if th.isl == true then
ape(
actor_any_collide,
{
evid = ee.edsp,
xpunch = tx,
ypunch = ty,
}
)
end
end
if th.isl == false then
if any_collide == false then
local offset_x = 4
if tvx < 0 then
offset_x = -offset_x
end
stc(
tx+offset_x,
ty-4,
1,8,
sre,
function(sensor)
if sensor._actor != th then
if tvx < 0 then
tx = max( tx, sensor.x+sensor.w+4)
else
tx = min( tx, sensor.x-4)
end
tvx = - tvx
any_collide = true
return true
end
return false
end
)
end
end
if th.isl == true then
stc(
tx-3,ty-3,6,6,sro,
function(sensor)
ape(
sensor._actor ,
{
evid = ee.edgc,
bounce = false
}
)
end
)
end
if any_collide == true and th.is_on_ground == true then
if th.isl == false then
th.fpp = 5
else
th.fpp = 1
end
end
if th.isl == true then
local lx = 4
if tvx < 0 then
lx = -lx
end
stc(
tx+lx-4,ty,
8,4,
sre,
function(sensor)
if sensor._actor != th then
th.fpp = 1
ape(
sensor._actor ,
{
evid = ee.edko,
bounce = true,
xpunch = tx
}
)
return true
end
end
)
else
stc(
tx-2,ty+4,
4,2,
sre,
function(sensor)
if sensor._actor != th then
ty = min( ty, sensor.y-4)
tvy = -1
end
end
)
end
end
function enemy_draw(th)
local hflip = false
if tvx > 0 then hflip = true end
local arg = th.asp
arg.x,
arg.y,
arg.fx,
arg.fy
=
tx,
ty,
hflip,
false
arg.frm_anim = flrshr(tx,1)
spx2( arg )
end
function edb(th,xpunch)
tvx = 0.09*(tx - xpunch)
tvy = -2.7
ty += tvy
end
function enud_crushed(th)
if acc(th) == true then
return
end
th.cnt_crushed += 1
if th.cnt_crushed >= 3 then
if th.is_on_ground == true then
if th.isl == false then
tvx *= 0.9
else
tvx *= 0.5
end
end
end
ensp(th)
if th.etp == ee.ets then
if th.cnt_crushed > 30 then
akl(th)
end
elseif
th.etp == ee.ett or
th.etp == ee.etk
then
local num_ev = acne(th)
for idx=0,num_ev-1 do
local ev = actor_ref_ev(th,idx)
if ev == nil then break end
if ev.evid == ee.edpk or ev.evid == ee.edso
then
if th.etp == ee.ett then
th.on_update,
th.on_draw,
th.on_touch,
th.isl,
tvx
=
enud,
enemy_draw,
entouch,
true,
2.0
if tx < ev.actor_kicker.x then
tvx = - tvx
end
ace(th)
return
elseif th.etp == ee.etk then
endko(th,ev)
ace(th)
return
end
elseif ev.evid == ee.edko then
endko(th,ev)
ace(th)
return
elseif
ev.evid == ee.edbs
then
if th.etp == ee.etk then
th.on_update,
th.on_draw,
th.on_touch,
th.is_crushed
=
enud,
enemy_draw,
entouch,
false
asp_ex_set( th.asp , enemytype2spr[ th.etp ] )
edb(th,ev.xpunch)
if tvx > 0 then
tvx = espe
else
tvx = -espe
end
ace(th)
return
else
endcr(th,ev)
edb(th,ev.xpunch)
ace(th)
return
end
end
end
ace(th)
enacm(th,
bor( sre , srec )
)
end
end
function edcsed(th)
local arg = th.asp
arg.x,
arg.y,
arg.fx,
arg.fy,
arg.frm_anim
=
tx,
ty,
false,
false,
flrshr( th.cnt_crushed , 6 )
spx2( arg )
end
function eko(th)
if acc(th) == true then
return
end
ensp(th)
end
function enedk(th)
local arg = th.asp
arg.x,
arg.y,
arg.fx
=
tx,
ty,
false
if th.etp == ee.etk then
if band(flrshr( ty , 2 ),1) != 0 then
arg.fy = true
else
arg.fy = false
end
else
arg.fy = true
end
spx2( arg )
end
function sres()
	sw = {
sensors_asbg = {},
cnt_bg = 0,
enb_touch_sensor = false,
}
ssrc()
end
function	ssrc()
	sw.gen_id,
	sw.sensors,
	sw.sensors_grid_cache,
sw.enb_touch_sensor
=1,{},{},false
end
_info_base = str2table(
{},
[[
x=0
y=0
w=0
h=0
vx=0
vy=0
]]
)
function	nsi()
	local info = {
id = sw.gen_id,
	}
merge(info,_info_base)
sw.gen_id += 1
	return	info
end
function	sad(info)
	add( sw.sensors, info )
local sx,ex,sy,ey
=
flrshr( info.x , 4 ),
flrshr( info.x + info.w , 4 ),
flrshr( info.y , 4 ),
flrshr( info.y + info.h , 4 )
for by=sy,ey do
for bx=sx,ex do
local hash = by*1024 + bx
if sw.sensors_grid_cache[ hash ] == nil then
sw.sensors_grid_cache[ hash ] = new_array()
end
apb(
sw.sensors_grid_cache[ hash ],
info
)
end
end
end
function	srab(info)
local bx,by = shr(info.x,3),shr(info.y,3)
	sw.sensors_asbg[by*1024+bx] = info
	sw.cnt_bg += 1
end
function	suab(info)
local bx,by = shr(info.x,3),shr(info.y,3)
	sw.sensors_asbg[by*1024+bx] = nil
	sw.cnt_bg -= 1
end
function	rvr( rc0 , rc1 )
	if (rc0.x+rc0.w<=rc1.x)	return	false
	if (rc1.x+rc1.w<=rc0.x)	return	false
	if (rc0.y+rc0.h<=rc1.y)	return	false
	if (rc1.y+rc1.h<=rc0.y)	return	false
	return	true
end
function	stc(x_,y_,w_,h_,register_,func_)
local results = stch(x_,y_,w_,h_,register_)
for ii=0,asz(results)-1 do
if func_( arf(results,ii) ) == true then
return
end
end
end
function	stch(x_,y_,w_,h_,register_)
	local rect0 = {x=x_,y=y_,w=w_,h=h_}
local results = new_array()
	asw( results, awa )
local xw,yh = x_+w_,y_+h_
local sx,ex,sy,ey,cache =
flrshr(x_,3),
flrshr(xw,3),
flrshr(y_,3),
flrshr(yh,3),
{}
for by=sy,ey do
for bx=sx,ex do
local sensor = sw.sensors_asbg[by*1024+bx]
if sensor != nil then
if band(sensor._register , register_) > 0 then
if cache[ sensor.id ] != true then
if rvr( rect0 , sensor ) == true then
apb(results,sensor)
cache[ sensor.id ] = true
end
end
end
end
end
end
local sx,ex,sy,ey,cache =
flrshr( x_,4),
flrshr( xw,4),
flrshr( y_,4),
flrshr( yh,4),
{}
for by=sy,ey do
for bx=sx,ex do
local hasharray = sw.sensors_grid_cache[by*1024+bx]
if hasharray != nil then
local size=asz( hasharray )
for ii=0,size-1 do
local sensor = arf(hasharray, ii)
if band(sensor._register , register_) > 0 then
if cache[ sensor.id ] != true then
if rvr( rect0 , sensor ) == true then
apb(results,sensor)
cache[ sensor.id ] = true
end
end
end
end
end
end
end
return	results
end
__gfx__
000000000007700000000111000000000077770000777700007777000000000044449999bb33bb33076677600000000000000000000000000000000000000000
000000000007100000011177110000000777aa700777aa700777aa700000007044449999bb33b933700770060000000000000000000000000000000000000000
00700700101770000001777777100000077777a7777777a7777777a70000000744449999444399396077006700000000000000a0000000000000000000000000
00077000111119a0000177777c710000f77777a717177717f7077707f000000744449999444499996770067600000000000000a0000000000000000000000000
000770001166199900117c77c7c10100f7777777f1717171f0007000f07000709999444499994444770067060000000000000090000000000000000000000000
00700700011119900117c777777c1710ff77777f0f77177f0000000f0f77077f9999444499994444700670060000000000000990000000000000001111000000
0000000096611000017c7777777777100ff777f00ff777f0000000f00ff777f09999444499994444606700660000000000000997700000000000007761100000
0000000011100000177777777777771100ffff0000ffff000000000000ffff009999444499994444067666600000000000007991171000000000071777100000
11111111511111111777777777777771000070008888888800000000000000000000000011111111111111110000000000007997771000000000071777111100
44414441111111151777777777777771000070001811181100000000000010000011100017bbbbbbbbbbbbb1000000000007779771111110000a999971111110
1111111151111111177c777777777c710006700081808180001111100001f1000019f100133333bb333333310000000000007711111110110000177777111011
f149f1491111111517c777c77c7777c1766667001008100801449f1000149f10001491001bbb33bb3b333b310000000000000771176110000001117777611011
914491445111111101cc77ccccc7cc7100d66667808180810144491001444100001441001bbb33bb3b33b3b10000000000009977777610000001107777771000
1111111111111115017ccc7777ccc77100066000081008100111110001441000001441001bbb33bb3b333b310000000000000997777991000001177777611100
49f149f1511111110111c71171111711000600008888888800000000001100000001110011111111111111110000000000000097676991100000099676991100
44914491111111150001111111001110000700001111111100000000000000000000000001111111111111100000000000000000000090000000999000999000
fffffffa044444440444444409999999000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4fffff7f477aaaaa4994444494444444000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
449999ff474aa4aa4941441494914914000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
449999ff4a4aa4aa4441441494914914000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
449999ff4aaaaaaa4444444494444444000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
449999ff4a4aaa4a4444444194914491000000111100000000000011110000000000000000000000000001111000000000000011110000000000000000000000
494444ff4aa444aa4444441494499914000007776110000000000777611000000000001111000000000077761100000000000777611000000000001111000000
9444444f4aaaaaaa4444444494444444000071777110000000007177711000000000077761100000000717771100000000007177711000000000077761100000
000aa100000991000004410077767665000071771110000000007177111000000000717771100000000717711110000000007177111000000000717771100000
00799a1000a44910009994107666655100a999971111111000a999971111111000007177111000000a9999911111110000a99997111111100000717711100000
0a9aa1a10949919104944141765655110000077777111011000007777611101100a99997111111100000077776110110000007777711101100a9999711111110
0a9aa1a1094991910494414166666551000000777761100000000777777110000000077776111011000000777761100000000077776110000000077777111011
0a9aa1a1094991910494414176565511000000997777100000009777777610000000077777111000000009977777100000009977777610000000077779911000
0a9aa1a1094991910494414165555551000009996761110000009967676991000000077779991000000099976761110000000997777991000000077777991000
00a11a10009119100041141065151511000000009901111000009900000991100000099769911100000000009901111000000097676991100000099666699100
000aa100000991000004410051111111000000099900000000000000000090000000999000000110000000099900000000000000000090000000999000011110
00111100000000000110000110001110000001001000111000001110111000000100011001111110880888888000000800000000000000000000000000000000
01777710000000011771000771017771111117117001777110017771777100001710177117777771770777777008000700000000000000000000000000000000
17000071111111177707100777177077777777777007707770070007700711117071700770007007700700077007000700000000000000000000000000000000
70000007777777777000710770770007710007007007000770070007700077777007000770007007700700077007000700000000000000000000000000000000
71000017070000077000071770070007071007007007001770070007700007777007100770007007700700077007000700000000000000000000000000000000
07111170000000007100007770070017007117007117007771171117710000007170711771117017788700877887888700010011110000000001001111100000
00777700000000000700000770000077000777007777007007777770770000000700077007770070777700777777777700110117777000000011001177770000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001100116771700000110011177717000
0111000001bbb33b333b3310000aa40000070000000700000047400000444400f00000f0f000000f0a79aaa00000000011110111771700001100011167717000
1177700001bbb33b33b3b31000779a400007000000070000049794000f4f4f400044400000000000a0700a0a000000000111111179999a0011111117799999a0
1177170001bbb33b333b33100a7aa4a400070000009a900049a7a940444944440449440000040000a0700a0a0000000000111177777000000111117777700000
1777170001bbb33b33b3b3100a9aa4a40077700009a7a9009aa7aa904f999f44f49994f0f044400faa79aa000000000000011777770000000011177777000000
1779999a01bbb33b333b33100a9aa4a47777a7707a777a70777777774449444404494400004440000a79aaa00000000000011699970000000601167777000000
1177770001bbb33b33b3b3100a9aa4a4007aa00009a7a9004aa7aa904f4f4f44044444000004000000700a0a0000000006011999660900006000199966000000
1177700001bbb33b333b331000a44a4000070000009a900009a7a94004444440f04f40f000000000a0700a0a0000000000001111999000000000999199990000
0000000001bbb33b33b3b310000aa4000007000000070000009794000044440000000000f00f000f0a79aaa00000000000060119990000000000011999000000
000000000000000000000000000000000000000000000000000bb00000eeee0000eeee0000000000000000000000000000000000000770000000000000000000
00000000077000000000000000000000000000000077700000b11b000eeeeee00eeeeee000000000000000000008870000000000008887000000000000000000
07700000017000000000000000000000000000000008870003bbbb30e77e77eee77e77ee00000000000000000000087000000000000088700000000000000000
917000009770000000000000000000000000000000808870bb1bb1bbe17e17eee17e17ee00000000000000000008008700000700078008800000000000000000
977900009999000000000000000070000000077077080870bbb11bbbe77e77eee77e77ee00000000008807000708000800000770778800800000000000000000
999900000099000000888770770807000000071017088880bb1bb1bbeeeeee88e88eeeee00eeee00098887707708880808880717178898800000001111000000
009900000099bb000801071017088070008887707708888077bbbb77088ee888888eee88177ee771880007171708988898888077700889800000077771100000
0999bb000097bb700808077077088880088100808000088007777770888eee0000eee888888ee888880800777008898888000080800088800000711761111110
9997bb709977bbb10881008080008880080808888800880080800188810080800080818881080800888800808000888888080888880088800000777711111111
0977bbb1077b1b1b08801888887008800808181818788000080801888178080000880811188888000880188888700880088818181878880000a9999711110011
077b1b1b07bbb1bb0188181818888800018118888880000008808811187087000880888188708700088818181888880008811887788000000000077776111011
07bbb1bb07bb1b1b00001888888000000008118888800000880808818808087088080888880808700000188778800000000888177180000007a9977777711010
07bb1b1b07b1bbb10000118888188000088801111118000088080188880808808808018888080880000018177188800008888717717800000aa9997777761010
07b1bbb1077bb7770018811111008800080080800088800088000770770808808800077077080880001887177178880008007717717780000999946767699100
077bb777997777990080808008000800080800800080800088000111110088008800011111008800008077177177880008080080008080000999990000099110
99777799099009990080008008008800000008000800800008880770770800000888077077080000008000800800080000000800080080000094940000009000
00000000000000000000a49090909090a490909090a490909090a490909090a490909090a490909090a490a490a49004a4900000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
04000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004400000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
3400000000000000000000000000000000000000000000000000000000000034000000000000002600660000000002b400000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
a4909090909090909000000000000000000000000000000000000000000000340000003333333333330235353535020000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000001400000000000000000002000066007600000000000001b40200000000000000001201010176120000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000003400000000000000000002333333333333333333000001000200000000000000120176010101120000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000012000000b400000000000000000002000000000000000000000001000200000000000012010101010176120000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000002000000000000760076007601000200000000001201760101010101120000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000002402000033333333333333333301000200000000120101010101010176120000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000076000000000000000000003402000000000000000000000001000200040012017601010101010101120000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000120012001200000003030303030300b402000000007600760076000001000200440201010101010101010176010000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000002333333333333123333000001000200a40202020202020202020202020200000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
14000000000000002400000000000000003402000000000000000000000001000200000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
34000000000000003400000000000000003402000076000076000076006600000200004400000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
a490909090909090a49090909090909090a490909090909090909090909090000200004400000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000007400000000000000009400000000000000000000000000000000004400000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000340000840000000000340000000000a0a0a000a0a0a000a00000a0b400000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000510000353535b40000341212121212b40000000000a000a000a0000000a0a000a00000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000011212000000b40000000000000000040000a0a0a000a0a0a000a0a0a0a00000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
7400000000000000840090940000000000040000540000a0000000a0000000a000a0a00000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
3400000000000000349080347676767676549000b40000a0000000a0a0a000a00000a00000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
a490909090909090a48080a49090909090a480900044000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000009400000044000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000004400000044000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000003535353535b4000000a4909090909090909090909090909000000000000000000000000000000000000000000111111111111110
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001666666661666771
00000000000000000000000001010101010000000000000000000000000000008400000000000000000000000000000000000000000000001166166666666671
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001666666166166661
00000000000000000000000000000000000000000000440000000000000000008400000000000000000000000000000000000000000000001611666666661161
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001661116666111661
0000000000000000000000940000000000440000000084840000000000000000b400000000000000000000000000000000000000000000001666111111116661
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001656111111116611
00000000000000000000004476767676768490909090b48490909090909090900000000000000000000000000000000000000000000000001666117117116661
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001566117117116161
0000000000000000000000a49090909090a48080808000a480808080808080800000000000000000000000000000000000000000000000001666611661166661
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001516666666661661
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001666611111166611
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001561116666111661
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111666666661111
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111111111111110
__gff__
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
__map__
4000404100414200004200000000400000000000000000004900000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4a004b4a094b4a00004b00000000410000000000000000004900004400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4400000000440000000000000000480000000000180000004b00004700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4a000000004b00106653625353104b00000000161016000000191a4b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000021212121212121000000000010201000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000003333333333000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000049101010111010104400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000049101010111010104700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000000000000400000000000000004a333333333333334a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000010102110101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4100200000000000000000002000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4800205353535304530453532053000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4a08080808080808080808080808000000004100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000040000000000000004000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000040000000000000004900000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000049000030303000004b00000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000010000067000000004b000033333300000000000000000000000000000000000000004500000000460047004800490000400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000200000000000000000006200000000000000000000420000000000000000004200000000420042004200420000410000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4000005320202053000041000021212100000000000000005300420000000000000000664b000000004b664b664b664b000a4b0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
40000020202020200000400000000000000000000000005300204b000000000033333333002153002100200010002000000a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
491020202020202020104910000066000010000000005300202000000000000000000000002100532100200010002000000a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4a2020202020202020204a20202020202020000000410020202000000000000000000000002153002100200010002000000a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
480000000000000000480000000000000000000000422020202000000000000000000000002100532100200010002000000a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4a33333333333333334b00000000000000000000204b2020202000000000000000000000002153002100200010002000000a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000000000000000470000000000000000002020002020202000000000000000000000002100532100200010002000000a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4709090909090909094b0000000000004000202020002020202000000000004300000000002153002100200010002000000a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
4a0808080808080808000000000000004220202020002020202000000000004200000000002100532100200010002000000a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000000000000000400000000000204b20202020002020202000200000004b00000000002153002100200010002000000a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000000000000000430000000020200020202020002020202000202000000000000000002100532100200010002000400a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000067006700004b4000002020204120202020422020202043202020004500000000462153002147204810492000400a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000010101021101000004200202020204220202020422020202042202020204200000000422100530042204210420000410a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
__sfx__
000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
__music__
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
00 41424344
