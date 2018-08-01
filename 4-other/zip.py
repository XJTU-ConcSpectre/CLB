import os
import os.path
import shutil

rootdir = "C:\\clb\\2-host+malicious code\\"

import zipfile
#打包目录为zip文件（未压缩）
def make_zip(source_dir, output_filename):
    zipf = zipfile.ZipFile(output_filename, 'w')    
    pre_len = len(os.path.dirname(source_dir))
    for parent, dirnames, filenames in os.walk(source_dir):
        for filename in filenames:
            pathfile = os.path.join(parent, filename)
            arcname = pathfile[pre_len:].strip(os.path.sep)     #相对路径
            zipf.write(pathfile, arcname)
    zipf.close()


for parent, dirnames, filenames in os.walk(rootdir):
    for filename in filenames:
        if "makefile" == filename:
            print(os.path.join(parent, filename))
            os.chdir(parent)
            os.system('make')

for parent, dirnames, filenames in os.walk(rootdir):
    for dirname in dirnames:
        if "statistic" == dirname:
            print(os.path.join(parent, dirname))
            shutil.rmtree(os.path.join(parent, dirname))
        if "programme" == dirname:
            print(os.path.join(parent, dirname))
            if os.path.exists(os.path.join(parent, dirname+".zip"))
                os.remove(os.path.join(parent, dirname+".zip"))
            make_zip(os.path.join(parent, dirname),os.path.join(parent, dirname+".zip"))
            

