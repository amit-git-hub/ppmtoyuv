#include "ppmtoyuv.h"


////////////
//main
////////////
INT32 main(INT32 argc, INT8 *argv[])
{
	FILE *fp;
	
	INT8 out_fname[512];
	INT8 byte;
	UINT8 yuv_format;
	UINT8 r,g,b;
	UINT32 rows,cols,depth,i,j;
	INT32 color=1,temp;
	UINT8 *Yptr,*Cbptr,*Crptr;	
	
	////////////////
	// image read 
	////////////////
	
	if(argc==2)
	{
		if((argv[1][0]!='-')||(argv[1][1]!='-')||(argv[1][2]!='h')||(argv[1][3]!='e')||(argv[1][4]!='l')||(argv[1][5]!='p'))
		{
			print_usage();
		}
	}
	
	if(argc!=4)
	{
		print_usage();
	}
	
	if((argv[1][0]!='-')||(argv[1][1]!='f')||(argv[1][2]!='\0'))
	{
		print_usage();
	}
	
	if ((fp = fopen(argv[3],"rb"))==NULL)
	{
		printf("Error accessing %s\n",argv[3]);
		exit(-1);
	}
	
	yuv_format = argv[2][0];
	yuv_format -= 48;
	
	if(yuv_format>2)
	{
		printf("Unsupported output YUV format.\n");
		exit(-1);
	}
	
	j = 0;
	for(i=0; argv[3][i] != '\0';i++)
	{
		if(argv[3][i] == '/')
			j = i + 1;
	}
	
	i = 0;
	while(argv[3][j] != '\0')
	{
		argv[3][i] = argv[3][j];
		i++;
		j++;
	}
	argv[3][i] = argv[3][j];
	
	for(i=0;argv[3][i]!='\0';i++);
	
	if(i>4)
	{
		argv[3][i-4] = '\0';
	}
	
	byte = fgetc(fp);
	if(byte != 'P')
	{
		printf("Fatal error: 'P' not found. Invalid ppm file.\n");
		exit(-1);
	}
	
	byte = fgetc(fp);
	if(byte != '6')
	{
		printf("Fatal error: 'P' not followed by 6. Unsupported ppm file.\n");
		exit(-1);
	}
	
	byte = fgetc(fp);//newline
	
	//get rid of those damn comments!
	byte = fgetc(fp);
	if (byte=='#')
	{
		do{ byte = fgetc(fp); } while (byte != '\n');//tolerate crap
	}
	else
	{
		ungetc((INT32)byte,fp);//wow! no crap!
	}
	
	fscanf(fp,"%d %d\n%d",&cols,&rows,&depth);
	fgetc(fp);//the damn '\n' after depth!
	
	if(yuv_format==YUV_420)
	{
		if((cols&0x1) || (rows&0x1))
		{
			printf("Odd dimensions not supported for YUV 420 output.\n");
			printf("Use IrfanView to resize the image to even dimensions.\n");
			exit(-1);
		}
	}
	
	/////////////
	// mallocs //
	/////////////
	Yptr = (UINT8 *)malloc(rows*cols*sizeof(UINT8));
	if(Yptr==NULL)
	{
		printf("Error allocating memory.");
		exit(-1);
	}
	Cbptr = (UINT8 *)malloc(rows*cols*sizeof(UINT8));
	if(Cbptr==NULL)
	{
		printf("Error allocating memory.");
		exit(-1);
	}
	Crptr = (UINT8 *)malloc(rows*cols*sizeof(UINT8));
	if(Crptr==NULL)
	{
		printf("Error allocating memory.");
		exit(-1);
	}
	
	for(i=0;i<rows;i++)
	{
		for(j=0;j<cols;j++)
		{
			r = fgetc(fp);
			g = fgetc(fp);
			b = fgetc(fp);
			
			temp = (INT32)((0.299*r) + (0.587*g) + (0.114*b));
			temp = MAX(temp, 0);
			temp = MIN(temp, 255);
			Yptr[(i*cols)+j] = temp;
			
			temp = (INT32)((b - Yptr[(i*cols)+j])*0.565 + 128);
			temp = MAX(temp, 0);
			temp = MIN(temp, 255);
			Cbptr[(i*cols)+j] = temp;
			
			temp = (INT32)((r - Yptr[(i*cols)+j])*0.713 + 128);
			temp = MAX(temp, 0);
			temp = MIN(temp, 255);
			Crptr[(i*cols)+j] = temp;
		}
	}
	
	fclose(fp);
	
	sprintf(out_fname,"%s_%dx%d.yuv", argv[3],cols,rows);
	fp = fopen(out_fname,"wb");
	if(!fp)
	{
		printf("Error accessing %s,out_fname");
		exit(-1);
	}
	
	switch(yuv_format)
	{
	case YUV_400:
		fwrite(Yptr,sizeof(UINT8),cols*rows,fp);
		break;
		
	case YUV_444:
		fwrite(Yptr,sizeof(UINT8),cols*rows,fp);
		fwrite(Cbptr,sizeof(UINT8),cols*rows,fp);
		fwrite(Crptr,sizeof(UINT8),cols*rows,fp);
		break;
		
	case YUV_420:
		downscale_h2v2(Cbptr, cols, rows);
		downscale_h2v2(Crptr, cols, rows);
		
		fwrite(Yptr,sizeof(UINT8),cols*rows,fp);
		fwrite(Cbptr,sizeof(UINT8),((cols*rows)>>2),fp);
		fwrite(Crptr,sizeof(UINT8),((cols*rows)>>2),fp);
		break;
		
	default:
		printf("Unsupported YUV format.\n");
	}
	
	fclose(fp);
	
	free(Yptr);
	free(Cbptr);
	free(Crptr);
	
	printf("YUV Conversion complete.");
	
	return(0);
}

void print_usage()
{
	printf("ppmtoyuv --help\t:\tDisplay this message.\n");
	printf("Usage: ppmtoyuv -f [format_code] <inputfile>\n");
	printf("Format codes:\n");
	printf("YUV400\t\t0\n");
	printf("YUV444\t\t1\n");
	printf("YUV420\t\t2");
	exit(0);
}

void downscale_h2v2(UINT8 *h2v2,UINT32 cols, UINT32 rows)
{
	UINT32 i,j;
	UINT8 *h2v2_ptr;
	
	h2v2_ptr = h2v2;
	
	for(i=0;i<rows;i+=2)
	{
		for(j=0;j<cols;j+=2)
		{
			*h2v2_ptr++ = h2v2[(i*cols)+j];
		}
	}
}




