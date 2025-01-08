#!/usr/bin/env python3

import subprocess

def main():
    if subprocess.call(['gcc', '-o', 'eval_madv', 'eval_madv.c']) != 0:
        print('eval_madv complie fail')
        return 1
    if subprocess.call(['gcc', '-o', 'eval_pmadv', 'eval_pmadv.c']) != 0:
        print('eval_pmadv complie fail')
        return 1

    hint = 4 # madv_dontneed
    nr_measures = 100
    sz_mem = 256 * 1024 * 1024

    print('# <nr_emasures> %d' % nr_measures)
    print('# <hint> <sz_mem> <sz_madv> <pmadv_batch> <madv|pmadv> <latency> <normalized latency>')

    sz_madv_pg = 1
    while sz_madv_pg <= 1024:
        sz_madv = sz_madv_pg * 4096
        madv_latency = int(subprocess.check_output(
            ['./eval_madv', '%s' % hint, '%s' % sz_mem, '%s' % sz_madv, '1',
             '%s' % nr_measures]).decode())
        pmadv_batch = 'n/a'
        print('%s %s %s %s madv %s 1.0' %
              (hint, sz_mem, sz_madv, pmadv_batch, madv_latency))
        pmadv_batch = 1
        while pmadv_batch <= 1024:
            sz_p_madv = pmadv_batch * sz_madv
            if sz_p_madv > sz_mem:
                break
            latency = int(subprocess.check_output(
                ['./eval_pmadv', '%s' % hint, '%s' % sz_mem, '%s' % sz_madv, '%s' %
                 sz_p_madv, '%s' % nr_measures]).decode())
            print('%s %s %s %s pmadv %s %.5f' %
                  (hint, sz_mem, sz_madv, pmadv_batch, latency, latency / madv_latency))
            pmadv_batch *= 2
        sz_madv_pg *= 2

if __name__ == '__main__':
    main()
