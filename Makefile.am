
bin_PROGRAMS = tenebrae.sdl

tenebrae_sdl_LDADD = @MATHLIB@ @INETLIB@ -lGL -lGLU

tenebrae_sdl_CFLAGS = @CFLAGS@ -DGLQUAKE -DSDL


tenebrae_sdl_SOURCES =		\
	adivtab.h		\
	anorm_dots.h		\
	anorms.h		\
	asm_draw.h		\
	asm_i386.h		\
	block16.h		\
	block8.h		\
	bspfile.h		\
	cd_sdl.c		\
	cdaudio.h		\
	chase.c			\
	cl_demo.c		\
	cl_input.c		\
	cl_main.c		\
	cl_parse.c		\
	cl_tent.c		\
	client.h		\
	cmd.c			\
	cmd.h			\
	common.c		\
	common.h		\
	conproc.h		\
	console.c		\
	console.h		\
	crc.c			\
	crc.h			\
	cvar.c			\
	cvar.h			\
	draw.h			\
	host.c			\
	host_cmd.c		\
	input.h			\
	keys.c			\
	keys.h			\
	mathlib.c		\
	mathlib.h		\
	menu.c			\
	menu.h			\
	modelgen.h		\
	mpdosock.h		\
	net.h			\
	net_bsd.c		\
	net_bw.h		\
	net_dgrm.c		\
	net_dgrm.h		\
	net_loop.c		\
	net_loop.h		\
	net_main.c		\
	net_udp.c		\
	net_udp.h		\
	net_vcr.c		\
	net_vcr.h		\
	pr_cmds.c		\
	pr_comp.h		\
	pr_edict.c		\
	pr_exec.c		\
	progdefs.h		\
	progs.h			\
	protocol.h		\
	quakeasm.h		\
	quakedef.h		\
	r_part.c		\
	r_local.h		\
	r_shared.h		\
	render.h		\
	resource.h		\
	sbar.c			\
	sbar.h			\
	screen.h		\
	server.h		\
	snd_dma.c		\
	snd_mem.c		\
	snd_mix.c		\
	snd_sdl.c		\
	sound.h			\
	spritegn.h		\
	sv_main.c		\
	sv_move.c		\
	sv_phys.c		\
	sv_user.c		\
	sys.h			\
	sys_sdl.c		\
	vgamodes.h		\
	vid.h			\
	gl_vidsdl.c		\
	view.c			\
	view.h			\
	wad.c			\
	wad.h			\
	winquake.h		\
	world.c			\
	world.h			\
	zone.c			\
	zone.h			\
	$(GL_SRCS)		\
	$(GL_TENEBRAE_SRCS)

QUAKE_SRCS =			\
	d_edge.c		\
	d_fill.c		\
	d_ifacea.h		\
	d_iface.h		\
	d_init.c		\
	d_part.c		\
	draw.c			\
	d_scan.c		\
	d_sky.c			\
	d_sprite.c		\
	d_surf.c		\
	r_aclip.c		\
	r_alias.c		\
	r_bsp.c			\
	r_draw.c		\
	r_edge.c		\
	r_efrag.c		\
	r_light.c		\
	r_main.c		\
	r_misc.c		\
	r_sky.c			\
	r_sprite.c		\
	r_surf.c		\
	r_varsa.s		\
	r_vars.c		\
	screen.c		


X86_SRCS =			\
	snd_mixa.s		\
	sys_dosa.s		\
	d_copy.s		\
	d_draw.s		\
	d_draw16.s		\
	d_parta.s		\
	d_polysa.s		\
	d_scana.s		\
	d_spr8.s		\
	d_varsa.s		\
	math.s			\
	r_aclipa.s		\
	r_aliasa.s		\
	r_drawa.s		\
	r_edgea.s		\
	surf16.s		\
	surf8.s			\
	worlda.s

NONX86_SRCS =			\
	d_vars.c		\
	nonintel.c


NULL_SRCS =			\
	cd_null.c		\
	in_null.c		\
	net_none.c		\
	snd_null.c		\
	sys_null.c		\
	vid_null.c

DOS_SRCS =			\
	cd_audio.c		\
	dos_v2.c		\
	in_dos.c		\
	mplib.c			\
	mplpc.c			\
	net_bw.c		\
	net_comx.c		\
	net_dos.c		\
	net_ipx.c		\
	net_ipx.h		\
	net_ser.c		\
	net_ser.h		\
	snd_dos.c		\
	snd_gus.c		\
	sys_dos.c		\
	vid_dos.c		\
	vid_dos.h		\
	vid_ext.c		\
	vid_vga.c		\
	vregset.c		\
	vregset.h

WIN_SRCS =			\
	cd_win.c		\
	conproc.c		\
	in_win.c		\
	net_mp.c		\
	net_mp.h		\
	net_win.c		\
	net_wins.c		\
	net_wins.h		\
	net_wipx.c		\
	net_wipx.h		\
	snd_win.c		\
	sys_win.c		\
	sys_wina.s		\
	sys_wind.c		\
	vid_win.c

LNX_SRCS =			\
	cd_linux.c		\
	snd_linux.c		\
	sys_linux.c		\
	vid_svgalib.c		\
	vid_x.c

GL_LNX_SRCS =			\
	gl_vidlinux.c		\
	gl_vidlinuxglx.c

GL_WIN_SRCS =			\
	gl_vidnt.c

SUN_SRCS =			\
	snd_sun.c		\
	in_sun.c		\
	sys_sun.c		\
	vid_sunx.c		\
	vid_sunxil.c

NEXT_SRCS =			\
	snd_next.c

GL_SRCS =			\
	gl_draw.c		\
	gl_mesh.c		\
	gl_model.c		\
	gl_model.h		\
	gl_refrag.c		\
	gl_rlight.c		\
	gl_rmain.c		\
	gl_rmisc.c		\
	gl_rsurf.c		\
	gl_screen.c		\
	gl_test.c		\
	gl_warp.c		\
	gl_warp_sin.h		\
	glquake.h		\
	glquake2.h

GL_TENEBRAE_SRCS =		\
	glATI.h			\
	gl_aliasinstant.c	\
	gl_brushinstant.c	\
	gl_bumpdriver.c		\
	gl_bumpgf.c		\
	gl_bumpmap.c		\
	gl_bumpradeon.c		\
	gl_glare.c		\
	gl_md3.c		\
	gl_screenrect.c		\
	gl_shadow.c		\
	gl_svbsp.c		

EXTRA_DIST =			\
	WinQuake.dsp		\
	WinQuake.dsw		\
	WinQuake.mdp		\
	WinQuake.opt		\
	WinQuake.plg		\
	quake.ico		\
	winquake.rc		\
	$(QUAKE_SRCS)		\
	$(NULL_SRCS)		\
	$(DOS_SRCS)		\
	$(WIN_SRCS)		\
	$(LNX_SRCS)		\
	$(SUN_SRCS)		\
	$(NEXT_SRCS)		\
	$(GL_SRCS)		\
	$(GL_LNX_SRCS)		\
	$(GL_WIN_SRCS)

dist-hook:
	cp -rp macosx docs dxsdk gas2masm $(distdir)/

