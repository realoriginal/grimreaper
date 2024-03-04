import click
import mako.template

def callback_obf_hash_make( string : str ) -> int:
    """
    Converts a string into a DJB2 hash.
    """
    hash = 5381

    # Make the string uppercase and loop through each character
    for x in string.upper():
        # Hash the character
        hash = ( ( hash << 5 ) + hash ) + ord( x )

    # return the has
    return hash & 0xFFFFFFFF

def callback_obf_stra_make( string : str ) -> str:
    """
    Converts a string into an array for ANSI
    """
    array_chars = ''

    # Loop through each char
    for indx, char in enumerate( string ):
        array_chars = array_chars + f'"{string}"[{indx}], '

    # Return the completed string
    return f'( CHAR[ {len( string ) + 1} ] )' + '{  ' + f'{array_chars}' + ' 0x00 }';

def callback_obf_strw_make( string : str ) -> str:
    """
    Converts a string into an array for WIDE
    """
    array_chars = ''

    # Loop trhough each char
    for char in string:
        array_chars = array_chars + f"L'{char}', ".format( char = char );

    # Return the completed string
    return f'( WCHAR[ {len( string ) + 2} ] )' + '{ ' + f'{array_chars}' + ' 0x0000 }';

@click.command( no_args_is_help = True )
@click.option( '-f', '--file', help = 'Path to the mako template.', type = click.File( 'r+' ), required = True )
@click.option( '-o', '--output', help = 'Path to write the evaluated template.', type = click.File( 'w+' ), required = True )
def export_template( file, output ):
    """
    Uses mako to evaluate the incoming template prior to compilation
    to achieve obfuscation.
    """
    # create the template
    template = mako.template.Template( file.read() );

    # Render the template
    temprend = template.render( 
        obf_hash_make = callback_obf_hash_make,
        obf_stra_make = callback_obf_stra_make,
        obf_strw_make = callback_obf_strw_make,
    );

    # write the result to the disk
    output.write( temprend );

if __name__ in '__main__':
    # Create the incoming template
    export_template();
