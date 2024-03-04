#!/usr/bin/env python3
# -*- coding:utf-8 -*-
import pefile
import argparse

if __name__ in '__main__':
    # create the argument parser
    parser = argparse.ArgumentParser( description = 'Extracts a shellcode from the first section of the PE.' );
    parser.add_argument( '-f', '--file', metavar = '', required = True, help = 'Path to the source executable to extract from.', type = str );
    parser.add_argument( '-o', '--output', metavar = '', required = True, help = 'Path to store the resulting shellcode.', type = str );
    option = parser.parse_args();

    # Parse the input portable exdecutable
    pe_exe = pefile.PE( option.file );
    pe_sec = pe_exe.sections[0].get_data();

    # Search for the ending tag in the executable
    if pe_sec.find( b'ENDOFCODE' ) != None:
        # Extract the raw 'shellcode'
        sc_raw = pe_sec[ : pe_sec.find( b'ENDOFCODE' ) ];

        # write the output
        pe_out = open( option.output, 'wb+' );
        pe_out.write( sc_raw );
        pe_out.close();
    else:
        # Print an error!?
        raise Exception( 'ENDOFCODE tag is missing from the executable' );
