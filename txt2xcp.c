//This is a program written by Pascal aka SnailMath that converts any file (preferably a text
//file into an xcp file used by calculators like the fx-cp 400 aka Classpad II.
//
//I'm not afiliated with any company.
//

#include <stdio.h>
#include <stdint.h>

const char bin2hex[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

char verbose = 0;
char chgnew = 0;   		//Change newline character
unsigned char check = 0xa6;	//The checksum. I hate this part...
uint32_t len = 0;		//length of the content of the var


unsigned char filestructure[] = {
  0x56, 0x43, 0x50, 0x2e,   0x58, 0x44, 0x41, 0x54, 
  0x41, 0x00, 0x35, 0x66,   0x34, 0x64, 0x34, 0x33, 
  0x35, 0x33, 0x30, 0x35,   0x6d, 0x61, 0x69, 0x6e, 
  0x00, 0x30, 0x35, 0x66,   0x69, 0x6c, 0x65, 0x00, 
  0x30, 0x30, 0x30, 0x30,   0x30, 0x30, 0x33, 0x31, 
  0x6d, 0x61, 0x69, 0x6e,   0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,   0xff, 0xff, 0xff, 0xff, 
  0x66, 0x69, 0x6c, 0x65,   0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff,   0xff, 0xff, 0xff, 0xff,
  
  //0x48					//len1 in big endian
  0x00, 0x00, 0x00, 0x00,			//(len+0x12)&(~0x03)

  0x47, 0x55, 0x51, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 

  //0x59
  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,  //len1 in ascii big endian

  //0x61					//len2 in little endian
  0x00, 0x00, 0x00, 0x00,			//len+3

  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
int filestructure_len = 110;


int main(int argc, char* argv[]){

	if ( argc < 3){ // 3 means command and 2 args
		fprintf( stderr, "\n\
This is the program txt2xcp by Pascal (aka SnailMath).\n\
My YouTube Channel: https://www.youtube.com/channel/UC4MX6_QKFwnA2sG9eGwqcjA\n\
My GitHub: https://github.com/SnailMath\n\
\n\
Usage: %s [OPTIONS] SOURCE DEST\n\
\n\
Converts the file SOURCE to an .xcp file called DEST.\n\
\n\
Options:\n\
  -l       Convert newline Characters from \"\\r\\n\" (Windows) or \"\\n\"\n\
           (Linux) to \"\\r\" (The newline characters used by the calc). Use\n\
           this only for text, and don't use it for programs or binary data.\n\
  -v       Don't use \"-v\"!\n\
\n\
Examples:\n\
%s yourfile.bin newfile.xcp\n\
%s -l yourfile2.txt newfile2.xcp\n\
\n\
The xcp file is of the type program(text). The variable will be called \"file\".\n", argv[0], argv[0], argv[0]);	
//  -n NAME  Specify the name of the variable. Up to 8 characters. Only when you pay $1000. I am to lazy to add this function otherwise...
		return -1;
	}

	int i = 1 ; //iterate over every argument except the last two
	while (i<argc-2){

		//printf("Checking argument \"%s\".\n",argv[i]);
		if (argv[i][0] == '-' && argv[i][1] == 'l' && argv[i][2] == 0) {
			chgnew = 1; }
		
		if (argv[i][0] == '-' && argv[i][1] == 'v' && argv[i][2] == 0) {
			verbose = 1; }

		/*if (argv[i][0] == '-' && argv[i][1] == 'n' && argv[i][2] == 0) {
			varname = argv[++i]; //first increment, then use i
			if (i==argc-2){
				fprintf(stderr,"ERROR: Parameter \"-n\" is specified but no name is given.\n");
				return -1;
			}
		
		}*/

		i++;//Go to the next argument
	}


	//The file stuff
	FILE *in;
	FILE *out;

	//open input file //DONT FORGET THE "b" FOR BINARY MODE! I HATE \r\n!!!
	in = fopen(argv[argc-2],"rb"); //The first argument is the infile,
	if (in == NULL){		//When -n then the 2nd is the file.
		fprintf(stderr, "ERROR: Cannot open file \"%s\" for reading!\n",argv[argc-2]);
		return -1;
	}
	//open out file
	out = fopen(argv[argc-1],"wb");
	if (in == NULL){
		fprintf(stderr, "ERROR: Cannot open file \"%s\" for writing!\n",argv[argc-1]);
		return -1;
	}
	//Now both files are open.

	//out = stdout; //FOR DEBUG WRITE TO STDOUT


	//First put the header in the out file. we will come back later and
	//Change this, when we know the size...
	if (verbose){ printf("creating the header..\n");}
	fwrite (filestructure, 1, filestructure_len, out ); //The header

	//now copy the content of in to out. don't forget to count bytes 
	//and to calculate the check sum.

	if (verbose){ printf("copying the content..\n");}
	while (1){
		int ch = fgetc(in);
		if (chgnew){	
			if (ch == '\r'){
	//When windows-newline (\r\n), add \r to the file but ignore the following \n
				fputc('\r', out);
				check -= ch;
				len++;
				ch = fgetc(in);
				if (ch == '\n'){
					continue; //If it is \n, go to the next char
				}		//If something else, this will be added
			}			//further down..
			if (ch == '\n'){
				//When linux newline (\n), add \r to the file.
				ch = '\r';
				//further down, this will be added to the file
			}
		}
		if (ch == EOF ){
			break;
		}
		
		fputc(ch, out);
		check -= ch;
		len++;
	}
	if (verbose){ printf("Length of the content: %d\nChecksum: 0x%02x\n",(int)len,check);}
	//printf("\nCharacters in the var: %d; checksum now: 0x%x\n", (int)len, check);

	//The end of the var
	if (verbose){ printf("adding the end of the file..\n");}
	fputc(0x00, out); 
	fputc(0xff, out);

	//The 0-3 bytes to align everything:
	// ((0x01 - length) & 0x03 ) bytes
	

	//reuse int i
	i = (0x01 - len) & 0x03 ;
	while ( i > 0 ) {
		fputc(0x00, out);
		i--;
	}
 
	//Store the current position
	long position = ftell(out) ;
	//printf ("position=0x%x\n",(int)position);

	//Calculate len1
	if (verbose){ printf("calculating length, modifying header..\n");}
	uint32_t len1 = (len + 0x12) & (~0x03);
	//printf ("len=0x%04x, len1=0x%04x, len2=0x%04x\n\n",len,len1,len+3);
	//len1 = 0x1abcdef9; //DEBUG
	
	fseek(out, 0x48, SEEK_SET); 
	fputc(len1>>(8*3), out);   check -= len1>>(8*3);
	fputc(len1>>(8*2), out);   check -= len1>>(8*2);
	fputc(len1>>(8  ), out);   check -= len1>>(8  );
	fputc(len1       , out);   check -= len1       ;
	
	//printf("current checksum: 0x%x\n", check);
	if (verbose){ printf("Checksum: 0x%02x\ncalculating length again, modifying header again..\n",check);}

	fseek(out, 0x59, SEEK_SET); 
	
	fputc(bin2hex[(len1>>(4*7))&0x0f],out); 
	fputc(bin2hex[(len1>>(4*6))&0x0f],out); 
	fputc(bin2hex[(len1>>(4*5))&0x0f],out);
	fputc(bin2hex[(len1>>(4*4))&0x0f],out);
	fputc(bin2hex[(len1>>(4*3))&0x0f],out);
	fputc(bin2hex[(len1>>(4*2))&0x0f],out);
	fputc(bin2hex[(len1>>(4  ))&0x0f],out);
	fputc(bin2hex[(len1       )&0x0f],out);

	check-=len1>>(8*3);
	check-=len1>>(8*2);
	check-=len1>>(8  );
	check-=len1       ;
	

	//printf("current checksum: 0x%x\n", check);
	if (verbose){ printf("Checksum: 0x%02x\ncalculating another length, modifying the header a third time.\n",check);}

	//Calculate len2
	len += 3; //This is len2
	//fseek(out, 0x61, SEEK_SET); 
	fputc(len       , out);   check -= len       ;
	fputc(len>>(8  ), out);   check -= len>>(8  );
	fputc(len>>(8*2), out);   check -= len>>(8*2);
	fputc(len>>(8*3), out);   check -= len>>(8*3);

	if (verbose){ printf("Checksum: 0x%02x\nHeader is now done, now add the checksum to the end..\n",check);}



	//Go to the end and add the checksum.
	fseek(out, position, SEEK_SET); //Restore the position
	
	
	fputc(bin2hex[(check>>4)&0x0f], out);
	fputc(bin2hex[(check   )&0x0f], out);

	if (verbose){ printf("That's it, now closeing the files..\n");}

	fclose(out);
	fclose(in);
	
	if (verbose){ printf("Everything done!!!\n");}

	printf("File converted successfully.\n");

}

