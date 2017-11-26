#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

//Random key distribution mechanism
/*The key distribution consists of three phases:
1. Key pre-distribution: in this phase a large key-pool of K keys and their corresponding identities are generated.
For each meter within the AMI network, k keys are randomly drawn from the key pool. These k keys form a key ring for a 
smart meter
2. Shared-key discovery: in this phase, each meter finds out which neighbor shares a common key with itself by exchanging 
discovery message. If two neighboring meter share a common key then there is a secure link between these two nodes
3. path-key establishment : in this phase, a path key is assigned to pair of neighboring meter who do not share 
a common key but can be connected by two or more multi-hop secure links at the end of the shared-key discovery. Here we have used 
maximum of two multi-hop secure links.
*/
//Creating a structure for each smart meter
typedef struct 
{
  int x,y; //co-ordinates of the smart meters
  int *keyring; //key ring associated with each meter
  int phynbrsize; //number of physical neighbors of a meter based on the distance
  int keynbrsize; //number of key neighbors of a meter based on the shared keys
  int *phynbr; //This pointer holds the list of the physical neighbors of the meter
  int *keynbr; //This pointer holds the list of the key neighbors of the meter
} smart_meter;

//build an IDS...prob

int n;
int total_path_key_one_hop_count=0;//path key count for one hop secure links between two non-neighboring meters
int total_path_key_two_hop_count =0;//path key count for two hop secure links between two non-neignoring meters
//sensor sensor_node[1000000];
smart_meter meters[1000000];
float average_neighbour_size=0.0,total_key=0.0, total_physical=0.0;


/*This function generates random key ring and location based on the the key ring size (k) and the keypool size (K)
k keys are randomly drawn from the key pool. 
These k keys form a key ring for a smart meter.
*/

void generating_random_numbers(int keyring_size, FILE *fp2, int keypool_size)
{
  
  char str[15];
  int i, j, x, y, index, n1, n2,flag=0, key1, fkey_ring=0;
  time_t t;
  srand((unsigned) time(&t));
  sprintf(str, "%d", n);
  fprintf(fp2,"%s\n", str);
  //allocation of random location i.e. x and y co-ordinates and keyring to a smart meter
  for(i=0; i<n; ++i)
            {//This block of code is dedicated to random allocation of x and y co-ordinates
            n1 = rand() % 500 + 1;
            n2 = rand() % 500 + 1; 	
            flag=0;
            for(j=0; j<i; ++j)
                    {
                      if(meters[j].x == n1 && meters[j].y == n2)
                              {
                              	--i;
                                flag=1;
                                break;
                              }
                    }
            if(flag==0)
                  {
                      meters[i].x = n1;
                      sprintf(str, "%d", n1);
                      fprintf(fp2,"%s ", str);
                      meters[i].y = n2;
	              sprintf(str, "%d", n2);
                      fprintf(fp2,"%s\n", str);
                  } 
            //This block of code is dedicated to random allocation of key ring to smart meter          
            meters[i].keyring =(int*)malloc(keyring_size*sizeof(int));
            for(x=0; x<keyring_size; ++x)
                {
                  key1 = rand() % keypool_size + 1;
                  fkey_ring=0;
                  for(y=0; y<x; ++y)
                      {
                      if (*(meters[i].keyring + y) == key1) //this is to ensure that the keys in a key ring is unique to each other
                            {
                              fkey_ring=1;
                              --x;
                              break;
                            }    
                      }
                  if(fkey_ring==0)
                      *(meters[i].keyring + x) = key1;//if all the keys are unique then assign the key to the key ring..repeat for all keys in the key ring
                }
            }
  fclose(fp2);
}

/*
This function finds the key neighbors of a smart meter based on the random keys allocated to them. If any two keys in the key ring of two meters matches than they are added to the key neighbor list
*/

void finding_key_neighbours(int keyring_size)
{
  printf("EG scheme\nDistributing keys...\n");
  int key_neighbours[100000],i,flag=0;
	for(i=0; i<n; ++i)//for all the smart meters
	{ 
		meters[i].keynbrsize=0;//initialize the key neighbor size to be zero
		int x;
		for(x=0; x<meters[i].phynbrsize; ++x)//for all the physical neighbor meters of a meter
		{
			int physical_neighbour_id,y,k;
			physical_neighbour_id = *(meters[i].phynbr+x);
			flag=0;
			for (y=0 ; y < keyring_size; ++y)//for all the keys inside the keyring of a meter
			{
				for (k= 0; k < keyring_size; ++k)//for all the keys inside the keyring of a physical neighbor
				{
					if(*(meters[physical_neighbour_id].keyring+k)==*(meters[i].keyring+y))
					{
						key_neighbours[meters[i].keynbrsize] = physical_neighbour_id;
						++meters[i].keynbrsize;
						flag=1;
						break; //break out of the loop if one key of the phy neighbor matches with a key of the meter
					}
				}
				if(flag!=0) break;//similarly break out of the loop if any of the key of the meter matches with a key of the physical neighbor 
			}

                }
                int z;
                meters[i].keynbr=(int*)malloc(meters[i].keynbrsize*sizeof(int));
                //fill the keynbr list of the keyneighbors of the 
                for(z=0; z<meters[i].keynbrsize; ++z)
                {
                *(meters[i].keynbr+z) = key_neighbours[z];
                }
	}
}


void finding_physical_and_key_neighbours(int keyring_size)
{

	double distance ,total_distance=0.0,physical_neighbour_size=0.0;
	int i=0, x=0;
	for(i=0; i<n; ++i)
        {
        	int phy_neighbours[100000]; 
        	meters[i].phynbrsize=0;// initialize the physical neighbor size to be zero
        	int j;
        	for (j = 0; j < n; ++j)
        	{
        		if (i==j)
        		{
        			continue; //if both the nodes are same
        		}
        		int x1,x2,y1,y2;
        		x1=meters[i].x;
        		y1=meters[i].y;
        		x2=meters[j].x;
        		y2=meters[j].y;
        		distance = sqrt(pow(x1-x2,2) + pow(y1-y2,2));

        		if(distance<=25.0)
        		{
        			
        			phy_neighbours[meters[i].phynbrsize] = j;
        			++meters[i].phynbrsize;
        		}
                        if(j>i)
                        total_distance = total_distance + distance;
        	}
                physical_neighbour_size=physical_neighbour_size + meters[i].phynbrsize;
                meters[i].phynbr  =(int*)malloc(meters[i].phynbrsize*sizeof(int));
                int k;
                int phy_nbr_size=meters[i].phynbrsize;
                for(k=0; k<phy_nbr_size; ++k)
                {
                         *(meters[i].phynbr+k) = phy_neighbours[k];
                }

       }
average_neighbour_size = physical_neighbour_size/n;        
printf("Scaling communication range...\nAverage distance = %f\nCommunication range of sensor nodes = 25.00\nComputing physical neighbors...\n",total_distance/((n-1)*(n/2) ));   
printf("Average neighborhood size = %f\n", average_neighbour_size);
finding_key_neighbours(keyring_size);        
}




void simulating_path_key_establishment(int keyring_size,int keypool_size)
{
  int arr[10000], temp_physical[1000], one_hop[1000], one_hop_temp[1000],i, j , k, x, y, index, z, w;
  for(i=0; i<n; ++i) //for all the smart meters
  {
        int t=0,flag=0,one_hop_index = 0,one_t=0;
        for(x=0; x < meters[i].phynbrsize; ++x)//for all the physical neighbors of a smart meter
        {
     		arr[x] = *(meters[i].phynbr+x);
        }
        for(j=0; j<meters[i].keynbrsize; ++j)//for all the key neighbor of a smart meter
        {
      		int key_nbr_j=*(meters[i].keynbr+j);
      		for(k=0; k<meters[i].phynbrsize; ++k)
      		{
                	if( key_nbr_j==arr[k])
        		{
          			arr[k]=-1; //allocate -1 to the arr element if the physical and key neighbor of the meters are same
        		}

      		}
        }
        // all the arr element with -1 indicates that the physical neighbor of the meter are also the key neighbor
        index=0;
        for(z=0; z < meters[i].phynbrsize; ++z)//for all the physical neighbor of the node
        {
               if(arr[z]==-1)
                     continue; //do not do anything 
               temp_physical[index] = arr[z];//separate out an array of two non key neighbor from all the physical neighbor
               ++index;      
        }
        //one-hop-temporary array creation..finds all the physical neighbor which are one hop far in having a common key
        for(t=0; t<index; ++t) // for all the physical neighbors which are not the key neighbors
        {
               flag=0;
               int temp_keynbrsize = meters[i].keynbrsize;
               for(y=0; y < temp_keynbrsize; ++y) //for all the key neighbor of that non key neighbor
               {
        		int temp_yth_keynbr=*(meters[i].keynbr+y);//stores the id of the key neighbor of the meter
        		for(z=0; z < meters[temp_yth_keynbr].keynbrsize;++z) //search across all the key neighbor of the yth key neighbor of the meter
        		{
          			if((*(meters[temp_yth_keynbr].keynbr+z))==temp_physical[t])
          			{
              				flag=1;
              				one_hop[one_hop_index++]= temp_physical[t];
              				++total_path_key_one_hop_count;
              				break;
          			}

        		}
        		if(flag==1)
          			break;

      		}

         }
      //two-hop temporary array creation...finds all the physical neighbor which are two hop far in having a common key
      for(x=0; x<one_hop_index; ++x) //for all the one-hop key neihbor
      {
      	    for(y=0; y<index; ++y) //for all the physical neighbor which are not the key neighbors
            {
                  if(one_hop[x]!=temp_physical[y]) // if the one hop neighbor is not equal to the temp_physical
                        continue;
                  temp_physical[y]=-1;//allocate -1 to the temp_physical element if the physical and key neighbor of the meters are same

            }
      }
      one_t=0;
      for (x = 0; x < index; ++x)
      {
            if(temp_physical[x]>-1)
            {
                   one_hop_temp[one_t++]=temp_physical[x];// separate out an array of all the neighbor which are neither key neighbor or one hop key neighbor and store in one_hop_temp
            }

        /* code */
      }
      /* Two hop counts calculation */
      for(x=0; x<one_t; ++x)
      {
           flag=0;
           int temp_key_nbr_size=meters[i].keynbrsize;
           for(y=0; y<temp_key_nbr_size; ++y)
           {
                  int for_y_keynbr=*(meters[i].keynbr+y);
                  for(z=0; z<meters[for_y_keynbr].keynbrsize; ++z)
                  {
                          int for_z_keynbr=*(meters[for_y_keynbr].keynbr+z);
                          for(w=0; w<meters[for_z_keynbr].keynbrsize; ++w)
                          {
                                  if(*(meters[for_z_keynbr].keynbr+w)== one_hop_temp[x])
                          	  {
                                       flag=1;
                                       ++total_path_key_two_hop_count;
                                       break;
                                  }
                          }
                          if(flag!=0)
                                  break;

                  }
                  if(flag!=0)
                          break;
            }

      }

  }

}

void finding_network_connectivity (int keyring_size, int keypool_size)
{
     float num_one_hop, num_two_hop=0.0,temp, p_connect=0.0, p_1, p_2,pro=1.0, num, den, peg;
     int count=0,i;
     printf("Network Connectivities : \n");
     for(i=0; i<n; ++i)
     {
             total_key +=meters[i].keynbrsize;
             total_physical +=meters[i].phynbrsize;
     }
     printf("\nSimulated Average Network Connectivity\n%f\nTheoritical Network Connectivity\n", total_key/total_physical);
     for (i=0; i<keyring_size ;++i)
     {
           temp = keypool_size - i;
           num = temp - keyring_size;
           pro *= (num/temp);
     }
     //pro : probability that any pair of node do not possess even a single common key
     //p_connect: probability that any pair of node has atleast one common key
     p_connect = 1-pro;
     p_1 = pow((1-pow(p_connect,2)), average_neighbour_size);
     p_1 = (1 - p_connect) * p_1;
     p_1 = 1 - p_1;
     p_2 = pow((1-(p_connect * p_1)), average_neighbour_size);
     p_2 *=(1-p_1);
     printf("%f\n", p_connect);
     printf("Simulated Average Network Connectivity for one hop\n%f\n", (total_path_key_one_hop_count + total_key)/total_physical);
     printf("Theoretical Average Network Connectivity for one hop\n%f\n", p_1 );
     printf("Simulated Average Network Connectivity for two hop\n%f\n", (total_path_key_one_hop_count + total_path_key_two_hop_count + total_key)/total_physical);
     printf("Theoretical Average Network Connectivity for two hop\n%f\n",1 - p_2);
}


/*generating_random_XY( int keyring_size,FILE *fp,int keypool_size)
{
	char str[100];
	memset(str, '\0', sizeof(str));
	sprintf(str, "%d", n);
  	fprintf(fp,"%s\n", str);
  	time_t t;
	srand((unsigned) time(&t));
  	int i=0, j=0,x,y,flag=0, key1,n1, n2;
	for(i=0; i<n; ++i)
	{
		
		//genrate key from pool 
		for (x = 0; sensor_node[i].phynbrsize;++x)
		{
			int fkey_ring=0;
			key1 = rand() % keypool_size + 1;
			for(y=0; y<x; ++y)
            {
            	if (*(sensor_node[i].keyring + y) == key1)
            	{
            		fkey_ring=1;
            		--x;
            		break;             
                }
           }
           if(fkey_ring==0)
           {
           	*(sensor_node[i].keyring + x) = key1;
           }
		}
		flag=0;
		n1 = rand() % 500 + 1;
        n2 = rand() % 500 + 1;
		sensor_node[i].keyring =(int*)malloc(keyring_size*sizeof(int));
		for(j=0; j<i; ++j)
        {
        	if(sensor_node[j].x == n1 && sensor_node[j].y == n2)
        	{
        		--i;
        		flag=1;
        		break;
        	}
        }
        if(flag==0)
        {			
 			sensor_node[i].x = n1;
        	sprintf(str, "%d", n1);
        	fprintf(fp,"%s ", str);
        	sensor_node[i].y = n2;
        	sprintf(str, "%d", n2);
          	fprintf(fp,"%s\n", str);
        }
	}
fclose(fp);
return;	
}*/

/*
void printing_sensor_nodes(int keyring_size)
{
	printf("Printing internal Structure\n");
	int i;
	for (i = 0; i < n; ++i)
	{
		printf("Node %d:-\n%d %d\n",i,sensor_node[i].x, sensor_node[i].y );
		printf("Genrated Key ring:-\n");
		int x;
		for(x=0; x<keyring_size; ++x)
    	{
           printf("%d", *(sensor_node[i].keyring+x));
           printf(" ");
        }
        printf("\nPhysical neighbors size %d\n physical neighbors are:-\n", sensor_node[i].phynbrsize);
        int phy_neighbours_size=sensor_node[i].phynbrsize;
        for(x=0; x<phy_neighbours_size; ++x)
    	{
           printf("%d", *(sensor_node[i].phynbr+x));
           printf(" ");
        }
        int key_neighbours_size=sensor_node[i].keynbrsize;
       printf("\nKey neighbors size %d\nkey neighbors are:-\n",key_neighbours_size);
       for(x=0; x<key_neighbours_size; ++x)
	   {
	                  printf("%d", *(sensor_node[i].keynbr+x) );
	     			  printf(" ");
	   }
      printf("\n");
     printf("One key count %d\nTwo key count %d\n",total_path_key_one_hop_count,total_path_key_two_hop_count);
         
	}
}

*/
int main( int argc, char *argv[] )  
{
	if(argc==4)
	{
	printf("Enter the number of smart meters\n");
	int argument_2,argument_3;
	char argument_1[100],string[10000]="plot[0:500][0:500] \"";
	memset(argument_1, '\0', sizeof(argument_1));
	strcpy(argument_1,argv[1]);
	strcat(string, argument_1);
        strcat(string, "\" every ::1 using 1:2");
	scanf("%d",&n);//enter the number of smart meters
	FILE *fp1;
        fp1=fopen("report", "w");
        fprintf(fp1,"%s", string);
        fclose(fp1);
	FILE *fp;
        fp=fopen(argument_1, "w");
        if(fp==NULL) 
        {
        	perror("Error opening file.");
        }
        argument_2=atoi(argv[2]);//the keyring size (k)
        argument_3=atoi(argv[3]);//the keypool size (K)
        //key pre-distribution: phase 1
        generating_random_numbers(argument_2,fp,argument_3);
        //Shared-key discovery: phase 2
        finding_physical_and_key_neighbours(argument_2);
        //path key establishment: phase 3
        simulating_path_key_establishment(argument_2,argument_3);
        finding_network_connectivity(argument_2,argument_3);
        }

        else
        {
    	printf("invalid number of arguments\n");
    	exit(0);
        }
	system("gnuplot -p 'report'"); 
}

