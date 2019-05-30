handle SIGILL pass nostop noprint
break LoadYmm0Breakpoint
break LoadZmm0Breakpoint
break LoadK0Breakpoint
break HandleSigill
cont
set width 4096
p $k0
cont
p $ymm0
cont
p $zmm0
quit
