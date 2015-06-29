import czesl
import glob
import argparse
import re
import multiprocessing as mp

def extract_errors(file_prefix, level, complexity):
    w_file = file_prefix + '.w.xml'
    a_file = file_prefix + '.a.xml'
    b_file = file_prefix + '.b.xml'
    czesl_w = czesl.CzeSLW(w_file)
    czesl_a = czesl.CzeSLA(a_file)
    czesl_a.set_wref(czesl_w)
    czesl_b = czesl.CzeSLB(b_file)
    czesl_b.set_aref(czesl_a)
    print 'Writing the corpus errors to : ' + file_prefix+'.train.sen.txt'
    if level == 'a':
        czesl_a.write_errors_by_sentences(file_prefix, complexity)
    elif level == 'b':
        czesl_b.write_errors_by_sentences(file_prefix, complexity)



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Tool for creating error corpus for Korektor")
    parser.add_argument('input_dirs', nargs='+', help='List of directories containing data - separated by spaces')
    parser.add_argument('--parallel', help='Parallel execution in the multi-core machines', action='store_true')
    parser.add_argument('--level', help='Error level from which the errors are to be extracted. Value can be either "a" or "b"')
    parser.add_argument('--complexity', help='Easy errors or complex errors to be written for test files. Value can be either "easy" or "complex"')
    args = parser.parse_args()

    a_files = []

    for my_dir in args.input_dirs:
        a_files.extend(glob.glob(my_dir + '/*.a.xml'))

    files_pre = map(lambda x: re.sub(r'\.a\.xml$', '', x), a_files)

    if args.parallel:
        error_extractors = map(lambda fpre: mp.Process(target=extract_errors, args=(fpre, args.level, args.complexity)), files_pre)
        for x in error_extractors:
            x.start()
        for x in error_extractors:
            x.join()
    else:
        for fp in files_pre:
            extract_errors(fp, args.level, args.complexity)


