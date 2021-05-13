// This program converts any file (preferably a text file
// into an xcp file used by calculators like the fx-cp 400 aka Classpad II.
//
//
// This is a program written by Pascal aka SnailMath. 
//   My GitHub: https://github.com/SnailMath/txt2xcp
//   My Discord: https://discord.gg/XWd4yNEhsR
//   My YouTube Channel: https://www.youtube.com/channel/UC4MX6_QKFwnA2sG9eGwqcjA
//   My Email: snailmathprog@gmail.com
//
//
// I'm not afiliated with any company.
//
//

#include <stdio.h>
#include <stdint.h>

#define tohex(x) ((x)<= 9 ?(x)+'0':(x)-10+'a') //convert from a number (0-15) to the ascii hex representation.
#define tobin(x) ((x)<='9'?(x)-'0':(x)+10-'a') //convert from a ascii hex nibble to the binary number.
#define info(...) if(verbose)printf(__VA_ARGS__); //print this, only if verbose.
#define BIG_ENDIAN 1
#define LITTLE_ENDIAN 0

char verbose = 0;		//verbose output
char chgnew = 0;   		//change newline character
char padchar = 0;   		//character used for padding

unsigned char checksum = 0x00;	//The checksum. (8 bit)

uint32_t addr_len1;		//This will hold the address where we'll add len1 later.
uint32_t addr_len1asc;		//This will hold the address where we'll add len1asc later.
uint32_t addr_len2;		//This will hold the address where we'll add len2 later.

uint32_t len1;			//This is len1 (sum of the following _length)
#define	 len2_length 	   4	//The length of the field len2
#define	 block_zero_length 9	//The length of the field block_zero
uint32_t data_length 	 = 0;	//The length of the data
#define	 eof_length 	   2	//The length of the eof structure of an xcp file
int	 padding_length;	//The length of the padding
		
char* varname = "file";		//The name of the var
char* folname = "main";		//The name of the folder
uint8_t varname_len = 5;	//The length of the name of the var (including the 0x00)
uint8_t folname_len = 5;	//The length of the name of the folder (including the 0x00)

char* infile = 0;		//The name of the input file
char* outfile= 0;		//The name of the output file
char outputfilename[64]; 	//Buffer for output file name

//Writes to the output file that keep track of the checksum
void xcpwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream, unsigned char* checksum);
void xcpwriteHexByte(uint8_t  number, FILE *stream, unsigned char* checksum);
void xcpwriteHexLong(uint32_t number, FILE *stream, unsigned char* checksum);
void xcpputc(int ch, FILE *stream, unsigned char* checksum);
void xcpwriteBinLong(uint32_t number, char endian, FILE *stream, unsigned char* checksum);

/*

The files on the calculator are called variables (var for short) and they are placed into folders.

// XCP file structure:
 1 vcp		 10 byte - the text "VCP.XDATA" including the 0x00 terminator
 2 longnumber	  8 byte - (hex ascii) the number 0x5f4d4353
 3 folname_len	  2 byte - (hex ascii) length of the folder name + 1
 4 folname	2-9 byte - name of the folder including 0x00 terminator
 5 varname_len	  2 byte - (hex ascii) length of the name + 1
 6 varname	2-9 byte - name of the var including 0x00 terminator) (also called "filename")
 7 block_31	  8 byte - the text "00000031"
 8 folname2	 16 byte - name of the folder, padded with 0xff
 9 varname2	 16 byte - name of the var, padded with 0xff
10 len1		  4 byte - (binary big endian) The length of len2, block_zero, data, eof and padding combined, this has to be divisible by 4, see padding
11 block_guq	 13 byte - the text "GUQ" followed by ten bytes of 0xff
12 len1asc	  8 byte - (hex ascii) the value of len1 again, but in ascii hex, small letters
13 len2		  4 byte - (binary little endian) the length of the data + 3
14 block_zero	  9 byte - nine times 0x00
15 data		len byte - the binary data
16 eof		  2 byte - The file terminator 0x00 0xff
17 padding	0-3 byte - pad the data so len1 is a multiple of 4 [pad with (3-((len+2)&~0x03)) bytes] 
18 checksum	  2 byte - (hex ascii) the checksum (take 0x00 and subtract all byes, except hex-ascii values, these are subtracted as hexadecimal numbers.
*/

int main(int argc, char* argv[]){

	if ( argc < 2){ // If we didn't get any args, show this message.
		fprintf( stderr, "\n\
This is the program txt2xcp by SnailMath.\n\
My YouTube Channel: https://www.youtube.com/channel/UC4MX6_QKFwnA2sG9eGwqcjA\n\
My GitHub:  https://github.com/SnailMath\n\
My Discord: https://discord.gg/XWd4yNEhsR\n\
My Email:   snailmathprog@gmail.com\n\
\n\
Usage: %s [OPTIONS] SOURCE [DEST]\n\
\n\
Converts the file SOURCE to an .xcp file called DEST.\n\
\n\
Options:\n\
  -l       Convert newline Characters from \"\\r\\n\" (Windows) or \"\\n\"\n\
           (Linux) to \"\\r\" (The newline characters used by the calc). Use\n\
           this only for text, and don't use it for programs or binary data.\n\
  -n NAME  Specify the name of the variable\n\
  -d NAME  Specify the name of the folder\n\
  -o NAME  Specify the output filename, if not provided at the end\n\
  -v       Verbose the output\n\
  -p       Don't use it! Specify character used for padding (default \\x00)\n\
\n\
Examples:\n\
%s yourfile.bin newfile.xcp\n\
%s -l yourfile2.txt newfile2.xcp\n\
%s -l -o out.xcp -n hello -f main yourfile3.txt\n\
\n\
The xcp file is of the type program(text).\n\
\n\
Tip: You can drag and drop a file into this program.\n", argv[0], argv[0], argv[0], argv[0]);	
		return 0;
	}

	int i = 1 ; //iterate over every argument
	while (i<argc){
		// -l
		if (argv[i][0] == '-' && argv[i][1] == 'l' && argv[i][2] == 0) {
			chgnew = 1;
		}
		// -v
		else if (argv[i][0] == '-' && argv[i][1] == 'v' && argv[i][2] == 0) {
			verbose = 1;
		}
		// -p
		else if (argv[i][0] == '-' && argv[i][1] == 'p' && argv[i][2] == 0) {
			i++; //go to the next arg
			if (i==argc){ fprintf(stderr,"ERROR: You are stupid. If you say -p then you have to tell me a characer to use!\n");return -1;}
			padchar = (tobin(argv[i][0]) << 4) +  tobin(argv[i][1]);
		}
		// -n
		else if (argv[i][0] == '-' && argv[i][1] == 'n' && argv[i][2] == 0) {
			i++; //go to the next arg
			if (i==argc){
				fprintf(stderr,"ERROR: Parameter \"-n\" is specified but no name is given.\n");
				return -1;
			}
			varname = argv[i]; //first increment, then use i
			//count the number of characters of the varname including the 0x00
			varname_len = 1;
			for(int i=0;varname[i]!=0;i++)varname_len++;
			if(varname_len>9) varname_len=9, varname[8]=0; //trunkate the name to 8 characters
		}
		// -d
		else if (argv[i][0] == '-' && argv[i][1] == 'd' && argv[i][2] == 0) {
			i++;
			if (i==argc){
				fprintf(stderr,"ERROR: Parameter \"-d\" is specified but no name is given.\n");
				return -1;
			}
			folname = argv[i];
			//count the number of characters of the folname including the 0x00
			folname_len = 1;
			for(int i=0;folname[i]!=0;i++)folname_len++;
			if(folname_len>9) folname_len=9, folname[8]=0; //trunkate the name to 8 characters
		}
		// -o
		else if (argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == 0) {
			i++;
			if (i==argc){
				fprintf(stderr,"ERROR: Parameter \"-o\" is specified but no name is given.\n");
				return -1;
			}
			if (outfile!=0) { //If we already have an outfile
				fprintf(stderr,"ERROR: There are too many output filenames given. Please use just one ouput file!\n");
				return -1;
			}
			outfile = argv[i];
		}
		// unrecognized parameter
		else if (argv[i][0] == '-') {
			fprintf(stderr,"ERROR: Parameter \"%s\" is not a recognized argument.\n", argv[i]);
			return -1;
		}
		//filename
		else {
			//if this occures, we are not reading an argument, but a file name.
			if(infile==0){ // If we haven't got an infile jet, this is it.
				infile = argv[i];
			}else if (outfile==0) { //If we already have an infile name, this must be the outfile name
				outfile = argv[i];
			}else{ //if we already have the file names, give an error...
				fprintf(stderr,"ERROR: There are too many filenames given. Please use just one input and one ouput file!\n");
				return -1;
			}
		}

		i++; //Go to the next argument
	}

	//check if input and output files are given:
	if(infile==0){
		fprintf(stderr,"ERROR: There is no input file given!\n");
		return -1;
	}
	if(outfile==0){ //when no outfile name was specified, use the infile with a new extension.
		int i=0;
		while(i<64-4-1){ //we need space for the .xcp and for the 0x00 at the end of the filename.
			char c = infile[i];
			if(c==0) break;
			outputfilename[i]=c;
			i++;
		}
		outputfilename[i++]='.'; //Add the file extension
		outputfilename[i++]='x';
		outputfilename[i++]='c';
		outputfilename[i++]='p';
		outputfilename[i  ]= 0 ;

		outfile = outputfilename;
	}
	
	//The file stuff
	FILE *in;
	FILE *out;

	//open input file //DONT FORGET THE "b" FOR BINARY MODE! I HATE \r\n!!!
	in = fopen(infile,"rb");
	if (in == NULL){
		fprintf(stderr, "ERROR: Cannot open file \"%s\" for reading!\n",infile);
		return -1;
	}
	//open out file
	out = fopen(outfile,"wb");
	if (in == NULL){
		fprintf(stderr, "ERROR: Cannot open file \"%s\" for writing!\n",outfile);
		fclose(in); //close the infile before crashing...
		return -1;
	}
	//Now both files are open.


	// Now convert the file!

	info(  "Step 1: vcp\n"  );
	{
		char buf[] = "VCP.XDATA"; //This is terminated by 0x00.
		//write to the outfile (and keep track of our checksum...)
		xcpwrite (buf, 1, sizeof(buf), out, &checksum);
	}
	
	info(  "Step 2: longnumber\n"  );
	{
		//write to the outfile (and keep track of our checksum...)
		xcpwriteHexLong(0x5f4d4353, out, &checksum);
	}

	info(  "Step 3: folname_len\n"  );
	{
		xcpwriteHexByte(folname_len, out, &checksum);
	}
	
	info(  "Step 4: folname\n"  );
	{
		xcpwrite (folname, 1, folname_len, out, &checksum);
	}
	
	info(  "Step 5: varname_len\n"  );
	{
		xcpwriteHexByte(varname_len, out, &checksum);
	}
	
	info(  "Step 6: varname\n"  );
	{
		xcpwrite (varname, 1, varname_len, out, &checksum);
	}

	info(  "Step 7: block_31\n"  );
	{
		xcpwriteHexLong(0x00000031, out, &checksum);
	}

	info(  "Step 8: folname2\n"  );
	{
		char buf[16];
		for(int i=0; i<16 ; i++) //copy the folname into buf and pad with 0xff
			buf[i] = (i<(folname_len-1))?folname[i]:0xff;
		xcpwrite (buf, 1, sizeof(buf), out, &checksum);
	}
	
	info(  "Step 9: varname2\n"  );
	{
		char buf[16];
		for(int i=0; i<16 ; i++) //copy the varname into buf and pad with 0xff
			buf[i] = (i<(varname_len-1))?varname[i]:0xff;
		xcpwrite (buf, 1, sizeof(buf), out, &checksum);
	}

	info(  "Step 10: skip len1 (for now)\n"  );
	{
		addr_len1 = ftell(out); //save this address, we'll come back to that later.
		//just write 0's, we'll add this later.
		char buf[4] = {0,0,0,0};
		fwrite (buf, 1, sizeof(buf), out );
	}

	info(  "Step 11: block_guq\n"  );
	{
		char buf[13] = {'G','U','Q',0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
		xcpwrite (buf, 1, sizeof(buf), out, &checksum);
	}

	info(  "Step 12: skip len1asc (for now)\n"  );
	{
		addr_len1asc = ftell(out); //save this address, we'll come back to that later.
		//just write 0's, we'll add this later.
		char buf[8] = {0,0,0,0,0,0,0,0};
		fwrite (buf, 1, sizeof(buf), out );
	}

	info(  "Step 13: skip len2 (for now)\n"  );
	{
		addr_len2 = ftell(out); //save this address, we'll come back to that later.
		//just write 0's, we'll add this later.
		char buf[len2_length] = {0,0,0,0};
		fwrite (buf, 1, sizeof(buf), out );
	}

	info(  "Step 14: block_zero\n"  );
	{
		char buf[block_zero_length] = {0,0,0,0,0,0,0,0,0};
		xcpwrite (buf, 1, sizeof(buf), out, &checksum);
	}

	info(  "Step 15: copy the data\n"  );
	{
		while (1){
			int ch = fgetc(in);
			if (chgnew){	
				if (ch == '\r'){ //When windows-newline (\r\n), add \r to the file but ignore the following \n
					xcpputc('\r', out, &checksum); data_length++; //add \r to the xcp file
					ch = fgetc(in);
					if (ch == '\n')	continue; //If it is \n, go to the next char, else add this char further down to the xcp file
				}
				if (ch == '\n'){ //When linux newline (\n), add \r to the file.
					ch = '\r'; //further down, this will be added to the xcp file
				}
			}
			if (ch == EOF )break;
			//Add the char to the xcp file
			xcpputc(ch, out, &checksum); data_length++;
		}
		info("Length of the content: %ld length of input file: %ld\n",(long int)data_length,(long int)ftell(in));
	}

	info(  "Step 16: eof\n"  );
	{
		char buf[eof_length] = {0x00,0xff}; //The data terminater is 00ff
		xcpwrite (buf, 1, sizeof(buf), out, &checksum);
	}
	
	info(  "Step 17: padding "  );
	{
		//The length of len2, block_zero, data, eof and this padding combined has to be a multiple of 4.
		//because `4+9+len+2+padding = 0 mod 4` has to be true we can deduce that `4+9+len+2 = - padding mod 4` is also true.
		//This means that `-(4+9+len+2) = padding mod 4`. Mod 4 means the rest dividing by 4 aka the last 2 digits.
		padding_length = (0-(len2_length+block_zero_length+data_length+eof_length)) & 0x03;
		char buf[] = {padchar,padchar,padchar};
		xcpwrite (buf, 1, padding_length, out, &checksum);	
	
		info("(padding with %d bytes)\n", padding_length);
	}

	//now to the len1 and len2 we left out...	
	info(  "now Step 10: len1 "  );
	{
		fseek(out,addr_len1,SEEK_SET); //return to the address we saved
		//len1 is the length of len2, block_zero, data, eof and padding combined, this is divisible by 4 (see padding)
		len1 = len2_length + block_zero_length + data_length + eof_length + padding_length;
		xcpwriteBinLong(len1, BIG_ENDIAN, out, &checksum);
	
		info("len1 is %d\n",len1);
		
	}

	info(  "now Step 12: len1asc\n"  );
	{
		fseek(out,addr_len1asc,SEEK_SET); //return to the address we saved
		xcpwriteHexLong(len1, out, &checksum);
	}
	
	info(  "now Step 13: len2\n"  );
	{
		fseek(out,addr_len2,SEEK_SET); //return to the address we saved
		uint32_t len2 = data_length + 3;
		xcpwriteBinLong(len2, LITTLE_ENDIAN, out, &checksum);
	}
	
	info(  "Step 18: checksum\n"  );
	{
		fseek(out,0,SEEK_END); //return to the end
		//write the checksum we've calculated
		xcpwriteHexByte(checksum, out, &checksum);
	}

	info(  "Closeing the files..\n"  );
	{
		fclose(out);
		fclose(in);
	}

	printf("File converted successfully.\n");
}


//writes the data to the file and keeps track of the checksum.
void xcpwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream, unsigned char* checksum){
	//output to the file
	fwrite (ptr, size, nmemb, stream );
	//calculate the new checksum
	for(unsigned long i = 0; i < size*nmemb ; i++){
		*checksum -= *((unsigned char*)ptr+i);
	}
}

//writes a 32 bit hexadecimal number to the file and keeps track of the checksum.
void xcpwriteHexLong(uint32_t number, FILE *stream, unsigned char* checksum){
	xcpwriteHexByte((uint8_t)(number>>(3*8)), stream, checksum);
	xcpwriteHexByte((uint8_t)(number>>(2*8)), stream, checksum);
	xcpwriteHexByte((uint8_t)(number>>(1*8)), stream, checksum);
	xcpwriteHexByte((uint8_t)(number>>(0*8)), stream, checksum);
}

//writes a 8 bit hexadecimal number to the file and keeps track of the checksum.
void xcpwriteHexByte(uint8_t number, FILE *stream, unsigned char* checksum){
	//output to the file
	fputc(tohex((number>>4)&0x0f), stream);
	fputc(tohex((number   )&0x0f), stream);
	//calculate the new checksum
	*checksum -= number;
}

//writes a single character to the file and keeps track of the checksum.
void xcpputc(int ch, FILE *stream, unsigned char* checksum){ //this returns void, not int
	//output to the file
	fputc(ch, stream)	;
	//calculate the new checksum
	*checksum -= ch;
}

//writes a 32 bit binary number to the file and keeps track of the checksum.
void xcpwriteBinLong(uint32_t number, char endianness, FILE *stream, unsigned char* checksum){
	if(endianness==BIG_ENDIAN){ //endianness is the order in which the bytes are written.
		xcpputc((number>>(3*8))&0xff, stream, checksum);
		xcpputc((number>>(2*8))&0xff, stream, checksum);
		xcpputc((number>>(1*8))&0xff, stream, checksum);
		xcpputc((number>>(0*8))&0xff, stream, checksum);
	} else { //(endian==LITTLE_ENDIAN
		xcpputc((number>>(0*8))&0xff, stream, checksum);
		xcpputc((number>>(1*8))&0xff, stream, checksum);
		xcpputc((number>>(2*8))&0xff, stream, checksum);
		xcpputc((number>>(3*8))&0xff, stream, checksum);
	}
}

