import os
import os.path
import shutil

rootdir = "E:\\git\\malicious-code-conceal\\3_2_multi_programmes_detect\\3_2_multi_programmes_detect\\"

malicious_code_4_c = "E:\\git\\malicious-code-conceal\\2_bullmoose\\bullmoose_4_pthread_NoOutPut_111432.c"
malicious_code_4_h = "E:\\git\\malicious-code-conceal\\2_bullmoose\\bullmoose_4_pthread_NoOutPut.h"

appdir = "E:\\git\\malicious-code-conceal\\3_2_multi_programmes_detect\\3_2_multi_programmes_detect\\apps"
detectdir = "E:\\git\\malicious-code-conceal\\3_2_multi_programmes_detect\\apps.LCLB.detect.111432"

for parent, dirnames, filenames in os.walk(detectdir):
    for filename in filenames:
        if "makefile" == filename:
            # replace makefile
            print(os.path.join(parent, filename))

            shutil.copy(malicious_code_4_c,
                        os.path.join(parent, 'bullmoose.c'))
            shutil.copy(malicious_code_4_h,
                        os.path.join(parent, 'bullmoose.h'))

            infile = open(os.path.join(parent, filename), "r")
            outfile = open(os.path.join(parent, "temp_makefile"), "w")
            lines = infile.readlines()
            for line in lines:
                if line.startswith("include"):
                    outfile.write(
                        "include ..\\..\\..\\null_macros\\Makefile.config\n")
                else:
                    outfile.write(line)
            infile.close()
            outfile.close()
            os.remove(os.path.join(parent, filename))
            shutil.move(os.path.join(parent, "temp_makefile"),
                        os.path.join(parent, filename))
            os.chdir(parent)
            os.system('make')
            os.chdir('..\\..\\')
        # if "programme.exe" == filename:
