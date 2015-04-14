import czesl
import glob
import argparse
import sys
import itertools
import re

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Tool for creating error corpus for Korektor")
    parser.add_argument('input_dirs', nargs='+', help='List of directories containing data - separated by spaces')
    args = parser.parse_args()

    a_files = []

    for my_dir in args.input_dirs:
        a_files.extend(glob.glob(my_dir + '/*.a.xml'))

    files_pre = map(lambda x: re.sub(r'\.a\.xml$', '', x), a_files)

    for f_pre in files_pre:
        w_file = f_pre + '.w.xml'
        a_file = f_pre + '.a.xml'

        # print '**************************************'
        # print 'Errors in dataset : ' + w_file_pre
        # print '**************************************'
        czesl_w = czesl.CzeSLW(w_file)
        czesl_a = czesl.CzeSLA(a_file)
        czesl_a.determine_edge_path_format()
        czesl_a.set_wref(czesl_w)
        # if czesl_a.edge_path_format == 2:
        #    print a_file
        czesl_a.print_errors()




