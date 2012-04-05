/****************************************************
1. Compile using command

gcc -o viz vizfinal.c -I/usr/local/include/opencv -L/usr/local/lib -lcxcore -lcv -lcvaux -lhighgui -lml

2 .run 

./viz

3. input file name

lena.bmp

4.output file

output1.jpg  : After Applying First three Step of Canny Algorithm
output2.jpg  : After Applying All Four Step of Canny Algorithm

*****************************************************/





/************************************************
		*CANNY ALGORITHM*

    * Step 1: Apply a Gaussian blur    
    * Step 2: Find edge Gradient strength and direction
    * Step 3: Trace along the edges
    * Step 4: Non-maximum Suppression
**************************************************/

#include<stdio.h>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include<math.h>

void makeimage(char *); 			// Function for Draw image from Result Matrix
void Edge(int ,int ,int ,int , int);		// Function for Finding Edge
void suppress(int ,int ,int ,int ,int);		//Function for Non-maximum Suppression

int height,width,step;

int upperThreshold = 40;				// Gradient strength nessicary to start edge
int lowerThreshold = 20;				// Minimum Gradient strength to continue edge
int *Direction;						// Stores the edge direction of each pixel
int *Gradient;						// Stores the Gradient strength of each pixel
int Result[4000][4000];						//Result Matrix to store the final matrix



int main(int argv ,char* argc)
{
 
 int i,j,k;
 const uchar*  data;					// pointer to the unaligned origin of image data
 int channels; 
 char  name[50];
			
 IplImage *img = NULL;					
 gets(name);
 img=cvLoadImage(name,1);				// load an image 
 if(!img) { 
          printf("cannot open image error");
          exit(1);
          }
          
          
// get the image data
 height = img->height;  				// image height in pixels
 width = img->width;   					// image width in pixels
 step = img->widthStep/sizeof(uchar); 			// size of aligned image row in bytes
 channels = img->nChannels;    				// Number of color channels (1,2,3,4)
 data = (uchar *)img->imageData;

 printf("Height=%d\nWidth=%d\n",height,width);

// display pixel value of each pixel
/* for(i=0;i<height;i++)
 {	
	for(j=0;j<width;j++)
		printf("%d ",data[i*width+j]);
	printf("\n");
 }
*/
/************************************************
    * Step 1: Apply a Gaussian blur
    * Step 2: Find edge Gradient strength and direction
    * Step 3: Trace along the edges
    * Step 4: Non-maximum Suppression
**************************************************/



int newPixel;						// Sum pixel values for gaussian


//scan  Gaussian Mask (5*5 matrix)
int Gm[5][5]={
 		{2,4,5,4,2},
 		{4,9,12,9,4},
 		{5,12,15,12,5},
 		{4,9,12,9,4},
 		{2,4,5,4,2}
 	};					

//scan  Sobel operator (3x3 convolution masks) for estimating the Gradient in the x-direction
int Gx[3][3]={
 		{-1,0,1},
 		{-2,0,2},
 		{-1,0,1}
 	};				
	
//scan  Sobel operator (3x3 convolution masks) for estimating the Gradient in the y-direction
int Gy[3][3]={
 		{1,2,1},
 		{0,0,0},
 		{-1,-2,-1}
 	};			
	
// Convert the RGB image into Grayscale using Formula [Y = 0.3*R + 0.59*G + 0.11*B]
for(i=0;i<height;i++) {
for(j=0;j<width;j++) {
           Result[i][j]= (int)(0.3*(int)(data[i*step + j*channels +2])+0.59*(int)(data[i*step + j*channels +1])+0.11*(int)(data[i*step + j*channels +0]));
}
}

		
// Step:: 1  Apply a Gaussian blur


int row,row1,col,col1;

for (row=2;row<height-2;row++) 
{
	for (col=2;col<width-2;col++) 
	{
		newPixel = 0;
		for (row1=-2; row1<=2; row1++)
		{
			for (col1=-2; col1<=2; col1++) 
				newPixel +=Result[row+row1][col+col1]*Gm[2+col1][2+row1];				
		}
	Result[row][col]=newPixel/159;
	}
}


// Step :: 2    Find edge Gradient strength and direction


Direction = (int *)malloc(height*width*sizeof(int));
Gradient = (int *)malloc(height*width*sizeof(int));

int x,y;
double Angle;

for (row=1;row<height-1;row++) 
{
	for (col=1;col<width-1;col++) 
	{
			x = 0;
			y = 0;
			/* Calculate the sum of the Sobel mask times the nine surrounding pixels in the x and y direction */
			for (row1=-1; row1<=1; row1++)
			{
				for (col1=-1; col1<=1; col1++) 
				{
					x+=Result[row+row1][col+col1]*Gx[1+col1][1+row1];
					y+=Result[row+row1][col+col1]*Gy[1+col1][1+row1];
				}
			}	

			Gradient[row*width+col] = (int)(sqrt(pow(x,2) + pow(y,2)));	// Calculate Gradient strength			
			Angle = (atan2(x,y)/3.14159) * 180.0;		// Calculate actual direction of edge
			
			//printf("%d %d %lf\n",x,y,Gradient[row*width+col]);					
			
			/* Convert actual edge direction to approximate value */
			if (((Angle<22.5)&&(Angle>-22.5)) || (Angle>157.5) || (Angle< -157.5))
				Angle = 0;
			if (((Angle > 22.5) && (Angle<67.5)) || ((Angle<-112.5) && (Angle>-157.5)))
				Angle = 45;
			if ( ( (Angle>67.5) && (Angle<112.5)) || ((Angle<-67.5) && (Angle>-112.5)))
				Angle = 90;
			if ( ( (Angle>112.5) && (Angle<157.5)) || ((Angle < -22.5) && (Angle > -67.5)))
				Angle = 135;
				
			Direction[row*width+col] = Angle;		// Store the approximate edge direction of each pixel in one array
	}
}


//Step :: 3     Trace along the edges
int flag;					// Stores whether or not the edge is at the edge of the possible image


	/* Trace along all the edges in the image */
for (row=1;row<height-1;row++) 
{
	for (col=1;col<width-1;col++) 
	{
	
			flag =0;
			if (Gradient[row*width+col] > upperThreshold) {	
				/* Switch based on current pixel's edge direction */
				switch (Direction[row*width+col]){		
					case 0:
						Edge(0,1,row,col,0);
						break;
					case 45:
						Edge(1,1,row,col,45);
						break;
					case 90:
						Edge(1,0,row,col,90);
						break;
					case 135:
						Edge(1,-1,row,col,135);
						break;
					default :
						Result[row][col]=0;
						break;
				}
			}
			else {
				Result[row][col]=0;
			}	
		}
	}



	/* Suppress any pixels not changed by the edge tracing */
for (row=0;row<height;row++) 
{
	for (col=0;col<width;col++) 
	{

	// If a pixel's grayValue is not black or white make it black
			if((Result[row][col]!= 255) && (Result[row][col]!= 0))
				Result[row][col]= 0 ; 
			}
	}
	
makeimage("output1.jpg");
 	
// Step :: 4    Non-maximum Suppression 


	for (row = 1; row < height-1; row++) {
		for (col = 1; col < width-1; col++) {
			if (Result[row][col] == 255) {		// Check to see if current pixel is an edge
				/* Switch based on current pixel's edge direction */
				switch (Direction[row*width+col]) {		
					case 0:
						suppress( 1, 0, row, col, 0);
						break;
					case 45:
						suppress( 1, -1, row, col, 45);
						break;
					case 90:
						suppress( 0, 1, row, col, 90);
						break;
					case 135:
						suppress( 1, 1, row, col, 135);
						break;
					default :
						break;
				}
			}	
		}
	}
	

 makeimage("output2.jpg");
 
}// End of Main Function


// Function for Draw image from Result Matrix

void makeimage(char *outfile) {
     int i,j;
     IplImage* imgt=cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
     for(i=0;i<height;i++) 
     {
     	for(j=0;j<width;j++) 
     	{	
	((uchar *)(imgt->imageData + i*imgt->widthStep))[j*imgt->nChannels + 0]= (uchar)Result[i][j]; 
	}
     }
	if(!cvSaveImage(outfile,imgt,0)) { 
        	    printf("Could not save: \n");
             }
	cvReleaseImage(&imgt );
}

// Function for Finding Edge

void Edge(int xShift, int yShift, int row, int col, int dir)
{
	int newRow;
	int newCol;
	int flag = 0;

	/* Find the row and column values for the next possible pixel on the edge */
	if (yShift<0) {
		if (col>0)
			newCol =col+yShift;
		else
			flag = 1;
	} else if (col< width-1) {
		newCol = col+yShift;
	} else
		flag =1;		// If the next pixel would be off image, don't do the while loop
	if (xShift < 0) {
		if (row > 0)
			newRow = row + xShift;
		else
			flag = 1;
	} else if (row < height-1) {
		newRow = row + xShift;
	} else
		flag = 1;	
		
	while ( (Direction[newRow*width+newCol]==dir) && !flag && (Gradient[newRow*width+newCol] > lowerThreshold) ) {
		/* Set the new pixel as white to show it is an edge */

		Result[newRow][newCol]=255;
		if (yShift < 0) {
			if (newCol > 0)
				newCol = newCol + yShift;
			else
				flag = 1;	
		} else if (newCol < width-1) {
			newCol = newCol + yShift;
		} else
			flag = 1;	
		if (xShift < 0) {
			if (newRow > 0)
				newRow = newRow + xShift;
			else
				flag =1;
		} else if (newRow < height-1) {
			newRow = newRow + xShift;
		} else
			flag =1;	
	}	
}

//Function for Non-maximum Suppression

void suppress(int xShift, int yShift, int row, int col, int dir)
{
	
	int newRow=0 ,newCol=0,flag=0,count;
	int nonMax[width][3];		// Temporarily stores Gradients and positions of pixels in parallel edges
	int pxlCount = 0;		// Stores the number of pixels in parallel edges
	int max[3];			// Maximum point in a wide edge
	
	if (yShift < 0) {
		if (col > 0)
			newCol = col + yShift;
		else
			flag = 1;
	} else if (col < width-1) 
	{
		newCol = col + yShift;
	} else
		flag = 1;		// If the next pixel would be off image, don't do the while loop
	  if (xShift < 0) {
		if (row > 0)
			newRow = row + xShift;
		else
			flag = 1;
	   } else if (row < height-1) 
	   {
		newRow = row + xShift;
	   } else
		flag = 1;	
	/* Find non-maximum parallel edges tracing up */
	while ((Direction[newRow*width+newCol] == dir) && !flag && (Result[newRow][newCol] == 255)) {
		if (yShift < 0) {
			if (newCol > 0)
				newCol = newCol + yShift;
			else
				flag = 1;	
		} else if (newCol < width-1) {
			newCol = newCol + yShift;
		} else
			flag = 1;	
		if (xShift < 0) {
			if (newRow > 0)
				newRow = newRow + xShift;
			else
				flag = 1;
		} else if (newRow < height-1) {
			newRow = newRow + xShift;
		} else
			flag = 1;	
		nonMax[pxlCount][0] = newRow;
		nonMax[pxlCount][1] = newCol;
		nonMax[pxlCount][2] = Gradient[newRow*width+newCol];
		pxlCount++;
	
	}

	/* Find non-maximum parallel edges tracing down */
	flag = 0;
	yShift *= -1;
	xShift *= -1;
	if (yShift < 0) {
		if (col > 0)
			newCol = col + yShift;
		else
			flag = 1;
	} else if (col < width-1) {
		newCol = col + yShift;
	} else
		flag = 1;	
	if (xShift < 0) {
		if (row > 0)
			newRow = row + xShift;
		else
			flag = 1;
	} else if (row < height- 1) {
		newRow = row + xShift;
	} else
		flag = 1;	

	while ((Direction[newRow*width+newCol] == dir) && !flag && (Result[newRow][newCol] == 255)) {
		if (yShift < 0) {
			if (newCol > 0)
				newCol = newCol + yShift;
			else
				flag = 1;	
		} else if (newCol < width- 1) {
			newCol = newCol + yShift;
		} else
			flag = 1;	
		if (xShift < 0) {
			if (newRow > 0)
				newRow = newRow + xShift;
			else
				flag = 1;
		} else if (newRow < height- 1) {
			newRow = newRow + xShift;
		} else
			flag = 1;	
		nonMax[pxlCount][0] = newRow;
		nonMax[pxlCount][1] = newCol;
		nonMax[pxlCount][2] = Gradient[newRow*width+newCol];
		pxlCount++;

	}

	/* Suppress non-maximum edges */
	max[0] = 0;
	max[1] = 0;
	max[2] = 0;
	for (count = 0; count < pxlCount; count++) {
		if (nonMax[count][2] > max[2]) {
			max[0] = nonMax[count][0];
			max[1] = nonMax[count][1];
			max[2] = nonMax[count][2];
		}
	}
	for (count = 0; count < pxlCount; count++) {

		Result[(int)(nonMax[count][0])][(int)(nonMax[count][1])] = 0;
	}
}

 
