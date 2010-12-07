from xml.dom import minidom
from emit_omnilog import *

def make_cpuid_h(dom) :
    tempstring = "";
    tempstring = tempstring +'/*this file is auto generated by volk_register.py*/';
    tempstring = tempstring +'\n#ifndef INCLUDED_VOLK_CPU_H';
    tempstring = tempstring +'\n#define INCLUDED_VOLK_CPU_H\n\n';
    tempstring = tempstring + emit_prolog();
    tempstring = tempstring + '\n'

    tempstring = tempstring + "struct VOLK_CPU {\n"
    for domarch in dom:
        arch = str(domarch.attributes["name"].value);
        tempstring = tempstring + "    int (*has_" + arch + ") ();\n";
    tempstring = tempstring + "}volk_cpu;\n\n";

    tempstring = tempstring + "void volk_cpu_init ();\n"
    tempstring = tempstring + "unsigned int volk_get_lvarch ();\n"

    tempstring = tempstring + "\n";
    tempstring = tempstring + emit_epilog();
    tempstring = tempstring + "#endif /*INCLUDED_VOLK_CPU_H*/\n"
    
    return tempstring;
