import czesl
import glob
import argparse
import re
import multiprocessing as mp

def extract_errors(file_prefix):
    w_file = file_prefix + '.w.xml'
    a_file = file_prefix + '.a.xml'
    czesl_w = czesl.CzeSLW(w_file)
    czesl_a = czesl.CzeSLA(a_file)
    czesl_a.set_wref(czesl_w)
    print 'Writing the corpus errors to : ' + file_prefix+'.err.sen.txt'
    #czesl_a.write_errors(file_prefix)
    czesl_a.write_errors_by_sentences(file_prefix)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Tool for creating error corpus for Korektor")
    parser.add_argument('input_dirs', nargs='+', help='List of directories containing data - separated by spaces')
    args = parser.parse_args()

    a_files = []

    for my_dir in args.input_dirs:
        a_files.extend(glob.glob(my_dir + '/*.a.xml'))

    files_pre = map(lambda x: re.sub(r'\.a\.xml$', '', x), a_files)
    error_extractors = map(lambda fpre: mp.Process(target=extract_errors, args=(fpre,)), files_pre)

    for x in error_extractors:
        x.start()

    for x in error_extractors:
        x.join()


