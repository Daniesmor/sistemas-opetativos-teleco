(function(g){var window=this;'use strict';var Grb=function(a,b){g.T.call(this,{I:"button",Ma:["ytp-miniplayer-expand-watch-page-button","ytp-button","ytp-miniplayer-button-top-left"],W:{title:"{{title}}","data-tooltip-target-id":"ytp-miniplayer-expand-watch-page-button","aria-keyshortcuts":"i","data-title-no-tooltip":"{{data-title-no-tooltip}}"},V:[{I:"svg",W:{height:"24px",version:"1.1",viewBox:"0 0 24 24",width:"24px"},V:[{I:"g",W:{fill:"none","fill-rule":"evenodd",stroke:"none","stroke-width":"1"},V:[{I:"g",W:{transform:"translate(12.000000, 12.000000) scale(-1, 1) translate(-12.000000, -12.000000) "},
V:[{I:"path",W:{d:"M19,19 L5,19 L5,5 L12,5 L12,3 L5,3 C3.89,3 3,3.9 3,5 L3,19 C3,20.1 3.89,21 5,21 L19,21 C20.1,21 21,20.1 21,19 L21,12 L19,12 L19,19 Z M14,3 L14,5 L17.59,5 L7.76,14.83 L9.17,16.24 L19,6.41 L19,10 L21,10 L21,3 L14,3 Z",fill:"#fff","fill-rule":"nonzero"}}]}]}]}]});this.J=a;this.Qa("click",this.onClick,this);this.updateValue("title",g.jU(a,"Expandir","i"));this.update({"data-title-no-tooltip":"Expandir"});this.addOnDisposeCallback(g.hU(b.Md(),this.element))},Hrb=function(a){g.T.call(this,
{I:"div",
S:"ytp-miniplayer-ui"});this.og=!1;this.player=a;this.T(a,"minimized",this.yh);this.T(a,"onStateChange",this.ZR)},Irb=function(a){g.CV.call(this,a);
this.B=new g.LK(this);this.j=new Hrb(this.player);this.j.hide();g.sU(this.player,this.j.element,4);a.qg()&&(this.load(),g.Ou(a.getRootNode(),"ytp-player-minimized",!0))};
g.w(Grb,g.T);Grb.prototype.onClick=function(){this.J.kb("onExpandMiniplayer")};g.w(Hrb,g.T);g.k=Hrb.prototype;
g.k.KO=function(){this.tooltip=new g.WX(this.player,this);g.J(this,this.tooltip);g.sU(this.player,this.tooltip.element,4);this.tooltip.scale=.6;this.Wc=new g.fW(this.player);g.J(this,this.Wc);this.Ak=new g.T({I:"div",S:"ytp-miniplayer-scrim"});g.J(this,this.Ak);this.Ak.Da(this.element);this.T(this.Ak.element,"click",this.HJ);var a=new g.T({I:"button",Ma:["ytp-miniplayer-close-button","ytp-button"],W:{"aria-label":"Cerrar"},V:[g.cG()]});g.J(this,a);a.Da(this.Ak.element);this.T(a.element,"click",this.Vp);
a=new Grb(this.player,this);g.J(this,a);a.Da(this.Ak.element);this.Av=new g.T({I:"div",S:"ytp-miniplayer-controls"});g.J(this,this.Av);this.Av.Da(this.Ak.element);this.T(this.Av.element,"click",this.HJ);var b=new g.T({I:"div",S:"ytp-miniplayer-button-container"});g.J(this,b);b.Da(this.Av.element);a=new g.T({I:"div",S:"ytp-miniplayer-play-button-container"});g.J(this,a);a.Da(this.Av.element);var c=new g.T({I:"div",S:"ytp-miniplayer-button-container"});g.J(this,c);c.Da(this.Av.element);this.KZ=new g.iX(this.player,
this,!1);g.J(this,this.KZ);this.KZ.Da(b.element);b=new g.hX(this.player,this);g.J(this,b);b.Da(a.element);this.nextButton=new g.iX(this.player,this,!0);g.J(this,this.nextButton);this.nextButton.Da(c.element);this.Nj=new g.PX(this.player,this);g.J(this,this.Nj);this.Nj.Da(this.Ak.element);this.progressBar=new g.nX(this.player,this);g.J(this,this.progressBar);g.sU(this.player,this.progressBar.element,4);this.qJ=new g.T({I:"div",S:"ytp-miniplayer-buttons"});g.J(this,this.qJ);g.sU(this.player,this.qJ.element,
4);a=new g.T({I:"button",Ma:["ytp-miniplayer-close-button","ytp-button"],W:{"aria-label":"Cerrar"},V:[g.cG()]});g.J(this,a);a.Da(this.qJ.element);this.T(a.element,"click",this.Vp);a=new g.T({I:"button",Ma:["ytp-miniplayer-replay-button","ytp-button"],W:{"aria-label":"Cerrar"},V:[g.Uva()]});g.J(this,a);a.Da(this.qJ.element);this.T(a.element,"click",this.p$);this.T(this.player,"presentingplayerstatechange",this.Ud);this.T(this.player,"appresize",this.Jb);this.T(this.player,"fullscreentoggled",this.Jb);
this.Jb()};
g.k.show=function(){this.wf=new g.Au(this.Jw,null,this);this.wf.start();this.og||(this.KO(),this.og=!0);0!==this.player.getPlayerState()&&g.T.prototype.show.call(this);this.progressBar.show();this.player.unloadModule("annotations_module")};
g.k.hide=function(){this.wf&&(this.wf.dispose(),this.wf=void 0);g.T.prototype.hide.call(this);this.player.qg()||(this.og&&this.progressBar.hide(),this.player.loadModule("annotations_module"))};
g.k.xa=function(){this.wf&&(this.wf.dispose(),this.wf=void 0);g.T.prototype.xa.call(this)};
g.k.Vp=function(){this.player.stopVideo();this.player.kb("onCloseMiniplayer")};
g.k.p$=function(){this.player.playVideo()};
g.k.HJ=function(a){if(a.target===this.Ak.element||a.target===this.Av.element)g.DL(this.player.Rb())?this.player.pauseVideo():this.player.playVideo()};
g.k.yh=function(){g.Ou(this.player.getRootNode(),"ytp-player-minimized",this.player.qg())};
g.k.We=function(){this.progressBar.Ic();this.Nj.Ic()};
g.k.Jw=function(){this.We();this.wf&&this.wf.start()};
g.k.Ud=function(a){g.EG(a.state,32)&&this.tooltip.hide()};
g.k.Jb=function(){g.zX(this.progressBar,0,this.player.ob().getPlayerSize().width,!1);g.oX(this.progressBar)};
g.k.ZR=function(a){this.player.qg()&&(0===a?this.hide():this.show())};
g.k.Md=function(){return this.tooltip};
g.k.Wf=function(){return!1};
g.k.Jf=function(){return!1};
g.k.gk=function(){return!1};
g.k.vB=function(){};
g.k.yp=function(){};
g.k.Qs=function(){};
g.k.An=function(){return null};
g.k.QH=function(){return null};
g.k.bO=function(){return new g.oe(0,0)};
g.k.Wk=function(){return new g.Tr(0,0,0,0)};
g.k.handleGlobalKeyDown=function(){return!1};
g.k.handleGlobalKeyUp=function(){return!1};
g.k.Hr=function(a,b,c,d,e){var f=0,h=d=0,l=g.gs(a);if(b){c=g.Ju(b,"ytp-prev-button")||g.Ju(b,"ytp-next-button");var m=g.Ju(b,"ytp-play-button"),n=g.Ju(b,"ytp-miniplayer-expand-watch-page-button");c?f=h=12:m?(b=g.es(b,this.element),h=b.x,f=b.y-12):n&&(h=g.Ju(b,"ytp-miniplayer-button-top-left"),f=g.es(b,this.element),b=g.gs(b),h?(h=8,f=f.y+40):(h=f.x-l.width+b.width,f=f.y-20))}else h=c-l.width/2,d=25+(e||0);b=this.player.ob().getPlayerSize().width;e=f+(e||0);l=g.je(h,0,b-l.width);e?(a.style.top=e+"px",
a.style.bottom=""):(a.style.top="",a.style.bottom=d+"px");a.style.left=l+"px"};
g.k.showControls=function(){};
g.k.rq=function(){};
g.k.Yj=function(){return!1};
g.k.uu=function(){};
g.k.Pr=function(){};
g.k.Gm=function(){};
g.k.Fm=function(){};
g.k.Eq=function(){};
g.k.Bp=function(){};
g.k.ex=function(){};
g.k.RH=function(){return null};g.w(Irb,g.CV);g.k=Irb.prototype;g.k.onVideoDataChange=function(){if(this.player.getVideoData()){var a=this.player.getVideoAspectRatio(),b=16/9;a=a>b+.1||a<b-.1;g.Ou(this.player.getRootNode(),"ytp-rounded-miniplayer-not-regular-wide-video",a)}};
g.k.create=function(){g.CV.prototype.create.call(this);this.B.T(this.player,"videodatachange",this.onVideoDataChange);this.onVideoDataChange()};
g.k.Ck=function(){return!1};
g.k.load=function(){this.player.hideControls();this.j.show()};
g.k.unload=function(){this.player.showControls();this.j.hide()};g.BV("miniplayer",Irb);})(_yt_player);
