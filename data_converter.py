import struct
import argparse
import sys


if __name__ == "__main__":

    parser = argparse.ArgumentParser( description="Convert a binary double file into ASCII data files suitable for NIR to consume" )
    parser.add_argument( '-s', metavar='skipcount', type=int, action='store', default=0, dest='skipcount', help='How many doubles to skip over between read values' )
    parser.add_argument( '-a', metavar='averagecount', type=int, action='store', default=1, dest='averagecount', help='How many read values to average over between outputting' )
    parser.add_argument( '-d', metavar='dimensions', type=int, action='store', default=2, dest='dimensions', help='How many read values to average over between outputting' )
    args = parser.parse_args()

    cur_dimension = 0
    while True:
        tot_dbl = 0.0
        for i in range(args.averagecount):
            # Skip over the next 'skipcount' doubles
            for j in range(args.skipcount):
                binary_bytes = sys.stdin.buffer.read(8)
                if not binary_bytes:
                    sys.exit(0)
            # Read this double
            binary_bytes = sys.stdin.buffer.read(8)
            if not binary_bytes:
                sys.exit(0)
            dbl = struct.unpack( "<d", binary_bytes )[0]

            tot_dbl += dbl
        tot_dbl /= args.averagecount
        sys.stdout.write( str(tot_dbl) )
        if cur_dimension == args.dimensions-1:
            sys.stdout.write('\n')
        else:
            sys.stdout.write(' ')
        cur_dimension = (cur_dimension+1) % args.dimensions
