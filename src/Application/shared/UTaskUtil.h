//---------------------------------------------------------------------------

#ifndef UTaskUtilH
#define UTaskUtilH
//---------------------------------------------------------------------------

// these functions are to get block-randomized numbers
// they contain static variables, so be careful when using
// them at more than one place
int     GetBlockRandomizedNumber( int blocksize );
void    ShuffleBlocks( int blocksize );
float   ran1( long *idum );

#endif
