#!/usr/bin/env python3

import argparse
import subprocess

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sz_mem', type=int, default=256 * 1024 * 1024)
    parser.add_argument('--pg_madv', type=int, nargs='+',
                        help='number of pages to do madvise at once')
    parser.add_argument('--sz_pmadv_batch', type=int, nargs='+')
    args = parser.parse_args()

    hint = 4 # madv_dontneed
    sz_mem = args.sz_mem
    pg_madv = args.pg_madv
    if pg_madv is None:
        pg_madv = [1]
    sz_pmadv_batch = args.sz_pmadv_batch
    if sz_pmadv_batch is None:
        sz_pmadv_batch = []
        sz = 1
        while sz <= 1024:
            sz_pmadv_batch.append(sz)
            sz *= 2

    if subprocess.call(['gcc', '-o', 'eval_madv', 'eval_madv.c']) != 0:
        print('eval_madv complie fail')
        return 1
    if subprocess.call(['gcc', '-o', 'eval_pmadv', 'eval_pmadv.c']) != 0:
        print('eval_pmadv complie fail')
        return 1

    hint = 4 # madv_dontneed
    sz_mem = 256 * 1024 * 1024

    print('# <hint> <sz_mem> <sz_madv> <pmadv_batch> <madv|pmadv> <latency> <normalized latency>')

    for sz_madv_pg in pg_madv:
        sz_madv = sz_madv_pg * 4096
        madv_latency = int(subprocess.check_output(
            ['./eval_madv', '%s' % hint, '%s' % sz_mem, '%s' % sz_madv, '1'
             ]).decode())
        pmadv_batch = 'n/a'
        print('%s %s %s %s madv %s 1.0' %
              (hint, sz_mem, sz_madv, pmadv_batch, madv_latency))
        for pmadv_batch in sz_pmadv_batch:
            sz_p_madv = pmadv_batch * sz_madv
            if sz_p_madv > sz_mem:
                break
            latency = int(subprocess.check_output(
                ['./eval_pmadv', '%s' % hint, '%s' % sz_mem, '%s' % sz_madv,
                 '%s' % sz_p_madv]).decode())
            print('%s %s %s %s pmadv %s %.5f' %
                  (hint, sz_mem, sz_madv, pmadv_batch, latency, latency / madv_latency))

if __name__ == '__main__':
    main()
