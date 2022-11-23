//    teetail - tee standard input to standard output and a file, but limit file size
//    Copyright (C) 2022 Sergei Lewis
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

int usage() {
    fprintf(stderr, 
        "tee standard input to standard output and a file, but limit file size\n"
        "\n"
        "Usage: teetail -o filename -c buffer_size [-Pq] [-B block_size]\n"
        "\n"
        "teetail duplicates standard input to standard output\n"
        "and also writes the last buffer_size bytes to the named file\n"
        "when its standard input reaches EOF\n"
        "\n"
        "Arguments below are optional.\n"
        "-B block_size  teetail attempts to perform IO in block_size chunks.\n"
        "-P             if supplied, teetail tracks current progress\n"
        "               on standard error.\n"
        "-q             if supplied, teetail is quiet - *nothing* is echoed\n"
        "               to stdout.\n"
        "\n"
        "e.g.\nteetail -o last_log -c 1024\n"
    );
    return -1;
}

typedef long long int millisecond_t;
millisecond_t current_time_millis(void) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (((millisecond_t)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

typedef long long unsigned int byte_size_t;

int main( int argc, char **argv ) {
    byte_size_t requested_size = 0;
    size_t block_size = 1024*1024;
    size_t buffer_size = 0;
    char const *destfilename = NULL;
    int quiet = 0;
    int progress = 0;
    millisecond_t last_ptime = current_time_millis();
    millisecond_t const start_ptime = last_ptime;

    if( argc < 5 ) {
        return usage();
    }

    int arg = 1;
    while( arg < argc ) {
        if( argv[arg][0] != '-' ) {
            fprintf(stderr, "failed to parse argument: %s\n", argv[arg]);
            return usage();
        }

        switch(argv[arg][1]) {
            case 'o':
                if( ++arg >= argc )
                    return usage();
                destfilename = argv[arg];
                break;

            case 'c':
                if( ++arg >= argc )
                    return usage();
                requested_size = atoll(argv[arg]);
                buffer_size = (size_t)requested_size; 
                if( buffer_size != requested_size ) {
                    fprintf(stderr, "buffer size doesn't fit in size_t!");
                    return -1;
                }
                break;

            case 'q':
                quiet=1;
                break;

            case 'P':
                progress=1;
                break;

            case 'B':
                if( ++arg >= argc )
                    return usage();
                requested_size = atoll(argv[arg]);
                block_size = (size_t)requested_size; 
                if( block_size != requested_size ) {
                    fprintf(stderr, "block size doesn't fit in size_t!");
                    return -1;
                }
                break;

            default:
                fprintf(stderr, "failed to parse argument: %s\n", argv[arg]);
                return usage();
        }

        ++arg;
    }

    if( !buffer_size || !destfilename ) {
        return usage();
    }

    char *buffer = malloc(buffer_size);
    if( !buffer ) {
        fprintf(stderr, "error allocating buffer memory\n");
        return -1;
    }

    byte_size_t total = 0;

    if( progress && !quiet ) {
        fprintf(stderr, "\n");
    }

    int return_code = 0;
    for(;;) {
        size_t const head = total % buffer_size;
        size_t const available_space = buffer_size - head;
        size_t const bytes_to_read = block_size < available_space ? block_size : available_space;

        size_t const bytes_read = fread( &buffer[head], sizeof(char), bytes_to_read, stdin );
        total += bytes_read;

        if(progress) {
            millisecond_t ptime = current_time_millis();
            if( (ptime - last_ptime) > 250 )
            {
                char const * unit = "bytes";
                byte_size_t displayed_total = total;
                if( displayed_total > 1024 ) {
                    unit = "Kb";
                    displayed_total = displayed_total/1024;
                }
                if( displayed_total > 1024 ) {
                    unit = "Mb";
                    displayed_total = displayed_total/1024;
                }
                if( displayed_total > 1024 ) {
                    unit = "Gb";
                    displayed_total = displayed_total/1024;
                }
                double rate = displayed_total / ((ptime - start_ptime)/1000.0);
                fprintf( stderr, "\r%llu %s read, %6.2f %s/s          ", displayed_total, unit, rate, unit );
                fflush( stderr );
                last_ptime = ptime;
            }
        }

        if( !quiet ) {
            size_t const bytes_written = fwrite( &buffer[head], sizeof(char), bytes_read, stdout );
            if( ( bytes_written < bytes_read ) || fflush( stdout ) ) {
                fprintf(stderr, "error writing to stdout\n");
                return_code = -1;
                break;
            }
        }

        if( feof(stdin) )
            break;
    }

    FILE * destfile = fopen(destfilename, "wb");
    if( !destfile ) {
        fprintf(stderr, "error opening %s for writing\n", destfilename);
        return -1;
    }

    if( total > buffer_size ) {
        size_t const head = total % buffer_size;
        fwrite( &buffer[head], sizeof(char), buffer_size-head, destfile );
        fwrite( &buffer[0], sizeof(char), head, destfile );
    } else {
        fwrite( &buffer[0], sizeof(char), total, destfile );
    }
    fflush( destfile );
    if( ferror( destfile ) ) {
        fprintf(stderr, "error writing to %s\n", destfilename);
        return -1;
    }
    fclose( destfile );

    return return_code;
}