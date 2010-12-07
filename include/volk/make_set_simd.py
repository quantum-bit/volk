from xml.dom import minidom

def make_set_simd(dom) :
    tempstring = "";
    tempstring = tempstring +'dnl this file is auto generated by volk_register.py\n\n';

    tempstring = tempstring + "AC_DEFUN([_MAKE_FAKE_PROCCPU],\n";
    tempstring = tempstring + "[\n";
    tempstring = tempstring + "  AC_REQUIRE([GR_SET_MD_CPU])\n";
    tempstring = tempstring + "  AC_MSG_CHECKING([proccpu])\n";
    tempstring = tempstring + "  case \"$MD_CPU\" in\n";
    tempstring = tempstring + "  (x86)\n";
    tempstring = tempstring + "    case \"$MD_SUBCPU\" in\n";
    tempstring = tempstring + "    (x86)\n";
    tempstring = tempstring + "      if test -z \"`${CC} -o proccpu -I ./include/ -I./lib lib/volk_proccpu_sim.c lib/volk_cpu_x86.c lib/cpuid_x86.S 2>&1`\"\n";
    tempstring = tempstring + "        then\n";
    tempstring = tempstring + "          AC_MSG_RESULT(yes)\n";
    tempstring = tempstring + "          lv_PROCCPU=\"`./proccpu`\"\n";
    tempstring = tempstring + "          rm -f proccpu\n";
    tempstring = tempstring + "        else\n";
    tempstring = tempstring + "          AC_MSG_RESULT(no)\n";
    tempstring = tempstring + "          lv_PROCCPU=no\n";
    tempstring = tempstring + "      fi\n"
    tempstring = tempstring + "    ;;\n"
    tempstring = tempstring + "    (*)\n"
    tempstring = tempstring + "      if test -z \"`${CC} -o proccpu -I ./include/ -I./lib lib/volk_proccpu_sim.c lib/volk_cpu_x86.c lib/cpuid_x86_64.S 2>&1`\"\n";
    tempstring = tempstring + "        then\n";
    tempstring = tempstring + "          AC_MSG_RESULT(yes)\n";
    tempstring = tempstring + "          lv_PROCCPU=\"`./proccpu`\"\n";
    tempstring = tempstring + "          rm -f proccpu\n";
    tempstring = tempstring + "        else\n";
    tempstring = tempstring + "          AC_MSG_RESULT(no)\n";
    tempstring = tempstring + "          lv_PROCCPU=no\n";
    tempstring = tempstring + "      fi\n"
    tempstring = tempstring + "    ;;\n"
    tempstring = tempstring + "    esac\n"
    tempstring = tempstring + "  ;;\n";
    tempstring = tempstring + "  (powerpc)\n";
    tempstring = tempstring + "    if test -z \"`${CC} -o proccpu -I ./include/ lib/volk_proccpu_sim.c lib/volk_cpu_powerpc.c 2>&1`\"\n";
    tempstring = tempstring + "      then\n";
    tempstring = tempstring + "        AC_MSG_RESULT(yes)\n";
    tempstring = tempstring + "        lv_PROCCPU=\"`./proccpu`\"\n";
    tempstring = tempstring + "        rm -f proccpu\n";
    tempstring = tempstring + "      else\n";
    tempstring = tempstring + "        AC_MSG_RESULT(no)\n";
    tempstring = tempstring + "        lv_PROCCPU=no\n";
    tempstring = tempstring + "    fi\n"
    tempstring = tempstring + "  ;;\n";
    tempstring = tempstring + "  (*)\n";
    tempstring = tempstring + "    if test -z \"`${CC} -o proccpu -I ./include/ lib/volk_proccpu_sim.c lib/volk_cpu_generic.c 2>&1`\"\n";
    tempstring = tempstring + "      then\n";
    tempstring = tempstring + "        AC_MSG_RESULT(yes)\n";
    tempstring = tempstring + "        lv_PROCCPU=\"`./proccpu`\"\n";
    tempstring = tempstring + "        rm -f proccpu\n";
    tempstring = tempstring + "      else\n";
    tempstring = tempstring + "        AC_MSG_RESULT(no)\n";
    tempstring = tempstring + "        lv_PROCCPU=no\n";
    tempstring = tempstring + "    fi\n"
    tempstring = tempstring + "  ;;\n";
    tempstring = tempstring + "  esac\n";
    tempstring = tempstring + "])\n"

    for domarch in dom:
        if str(domarch.attributes["type"].value) != "all":
            arch = str(domarch.attributes["name"].value);
            flag = domarch.getElementsByTagName("flag");
            flag = str(flag[0].firstChild.data);
            tempstring = tempstring + "AC_DEFUN([_TRY_ADD_" + arch.swapcase() + "],\n";
            tempstring = tempstring + "[\n";
            tempstring = tempstring + "  LF_CHECK_CC_FLAG([-" + flag + "])\n";
            tempstring = tempstring + "  LF_CHECK_CXX_FLAG([-" + flag + "])\n";
            tempstring = tempstring + "])\n";

    tempstring = tempstring + "AC_DEFUN([LV_SET_SIMD_FLAGS],\n";
    tempstring = tempstring + "[\n";
    tempstring = tempstring + "  AC_REQUIRE([GR_SET_MD_CPU])\n";
    tempstring = tempstring + "  AC_SUBST(LV_CXXFLAGS)\n";
    tempstring = tempstring + "  indCC=no\n";
    tempstring = tempstring + "  indCXX=no\n";
    tempstring = tempstring + "  indLV_ARCH=no\n";
    tempstring = tempstring + "  AC_ARG_WITH(lv_arch,\n";
    tempstring = tempstring + "    AC_HELP_STRING([--with-lv_arch=ARCH],[set volk hardware speedups as space separated string with elements from the following list(";
    
    for domarch in dom:
        arch = str(domarch.attributes["name"].value);
        tempstring = tempstring + arch + ", "
    tempstring = tempstring[0:len(tempstring) - 2];
        
    tempstring = tempstring + ")]),\n";
    tempstring = tempstring + "      [cf_with_lv_arch=\"$withval\"],\n";
    tempstring = tempstring + "      [cf_with_lv_arch=\"\"])\n";
    if str(domarch.attributes["type"].value) == "all":
        arch = str(domarch.attributes["name"].value);    
        tempstring = tempstring + "  AC_DEFINE(LV_HAVE_" + arch.swapcase() + ", 1, [always set "+ arch + "!])\n";
    tempstring = tempstring + "  ADDONS=\"\"\n";
    tempstring = tempstring + "  _MAKE_FAKE_PROCCPU\n";
    tempstring = tempstring + "  if test -z \"$cf_with_lv_arch\"; then\n";
    tempstring = tempstring + "    cf_with_lv_arch=$lv_PROCCPU\n";
    
    tempstring = tempstring + "  fi\n";
    tempstring = tempstring + "  echo $cf_with_lv_arch\n";
    for domarch in dom:
        if str(domarch.attributes["type"].value) != "all":
            arch = str(domarch.attributes["name"].value);
            tempstring = tempstring + "  LV_HAVE_" + arch.swapcase() + "=no\n";

    tempstring = tempstring + "  case \"$MD_CPU\" in\n";
    tempstring = tempstring + "  (x86)\n"
    for domarch in dom:
        arch = str(domarch.attributes["name"].value);
        atype = str(domarch.attributes["type"].value);
        if atype == "x86":
            tempstring = tempstring + "    _TRY_ADD_" + arch.swapcase() + "\n";

    for domarch in dom:
        arch = str(domarch.attributes["name"].value);
        atype = str(domarch.attributes["type"].value);
        flag = domarch.getElementsByTagName("flag");
        flag = str(flag[0].firstChild.data);
        if atype == "x86":
            tempstring = tempstring + "    for i in $lf_CXXFLAGS\n"
            tempstring = tempstring + "    do\n"
            tempstring = tempstring + "      if test \"X$i\" = X-" + flag +"; then\n";
            tempstring = tempstring + "        indCXX=yes\n";
            tempstring = tempstring + "      fi\n"
            tempstring = tempstring + "    done\n"
            tempstring = tempstring + "    for i in $lf_CFLAGS\n"
            tempstring = tempstring + "    do\n"
            tempstring = tempstring + "      if test \"X$i\" = X-" + flag +"; then\n";
            tempstring = tempstring + "        indCC=yes\n";
            tempstring = tempstring + "      fi\n"
            tempstring = tempstring + "    done\n"
            tempstring = tempstring + "    for i in $cf_with_lv_arch\n"
            tempstring = tempstring + "    do\n"
            tempstring = tempstring + "      if test \"X$i\" = X" + arch + "; then\n";
            tempstring = tempstring + "        indLV_ARCH=yes\n"
            tempstring = tempstring + "      fi\n"
            tempstring = tempstring + "    done\n"
            tempstring = tempstring + "    if test \"$indCC\" == \"yes\" && test \"$indCXX\" == \"yes\" && test \"$indLV_ARCH\" == \"yes\"; then\n"
            tempstring = tempstring + "      AC_DEFINE(LV_HAVE_" + arch.swapcase() + ", 1, [" + arch + " flag set])\n";
            tempstring = tempstring + "      ADDONS=\"${ADDONS} -" + flag + "\"\n";
            tempstring = tempstring + "      LV_HAVE_" + arch.swapcase() + "=yes\n";
            tempstring = tempstring + "    fi\n"
            tempstring = tempstring + "    indCC=no\n"
            tempstring = tempstring + "    indCXX=no\n"
            tempstring = tempstring + "    indLV_ARCH=no\n"
        elif atype == "all":
            tempstring = tempstring + "      AC_DEFINE(LV_HAVE_" + arch.swapcase() + ", 1, [" + arch + " flag set])\n";
            tempstring = tempstring + "      LV_HAVE_" + arch.swapcase() + "=yes\n";
    tempstring = tempstring + "  ;;\n"
        
    tempstring = tempstring + "  (powerpc)\n"
    for domarch in dom:
        arch = str(domarch.attributes["name"].value);
        atype = str(domarch.attributes["type"].value);
        if atype == "powerpc":
            tempstring = tempstring + "    _TRY_ADD_" + arch.swapcase() + "\n";

    for domarch in dom:
        arch = str(domarch.attributes["name"].value);
        atype = str(domarch.attributes["type"].value);
        flag = domarch.getElementsByTagName("flag");
        flag = str(flag[0].firstChild.data);
        if atype == "powerpc":
            tempstring = tempstring + "    for i in $lf_CXXFLAGS\n"
            tempstring = tempstring + "    do\n"
            tempstring = tempstring + "      if test \"X$i\" = X-" + flag +"; then\n";
            tempstring = tempstring + "        indCXX=yes\n";
            tempstring = tempstring + "      fi\n"
            tempstring = tempstring + "    done\n"
            tempstring = tempstring + "    for i in $lf_CFLAGS\n"
            tempstring = tempstring + "    do\n"
            tempstring = tempstring + "      if test \"X$i\" = X-" + flag +"; then\n";
            tempstring = tempstring + "        indCC=yes\n";
            tempstring = tempstring + "      fi\n"
            tempstring = tempstring + "    done\n"
            tempstring = tempstring + "    for i in $cf_with_lv_arch\n"
            tempstring = tempstring + "    do\n"
            tempstring = tempstring + "      if test \"X$i\" = X" + arch + "; then\n";
            tempstring = tempstring + "        indLV_ARCH=yes\n"
            tempstring = tempstring + "      fi\n"
            tempstring = tempstring + "    done\n"
            tempstring = tempstring + "    if test \"$indCC\" = yes && test \"indCXX\" = yes && \"indLV_ARCH\" = yes; then\n"
            tempstring = tempstring + "      AC_DEFINE(LV_HAVE_" + arch.swapcase() + ", 1, [" + arch + " flag set])\n";
            tempstring = tempstring + "      ADDONS=\"${ADDONS} -" + flag + "\"\n";
            tempstring = tempstring + "      LV_HAVE_" + arch.swapcase() + "=yes\n";
            tempstring = tempstring + "    fi\n"
            tempstring = tempstring + "    indCC=no\n"
            tempstring = tempstring + "    indCXX=no\n"
            tempstring = tempstring + "    indLV_ARCH=no\n"
        elif atype == "all":
            tempstring = tempstring + "      AC_DEFINE(LV_HAVE_" + arch.swapcase() + ", 1, [" + arch + " flag set])\n";
            tempstring = tempstring + "      LV_HAVE_" + arch.swapcase() + "=yes\n";
    tempstring = tempstring + "  ;;\n"
    tempstring = tempstring + "  esac\n"
    tempstring = tempstring + "  LV_CXXFLAGS=\"${LV_CXXFLAGS} ${ADDONS}\"\n"
    tempstring = tempstring + "])\n"
   
    return tempstring;
                
                
        
