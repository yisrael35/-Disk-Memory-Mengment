#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

using namespace std;

#define DISK_SIZE 256


/*yisrael bar 19-23/07/20 
id 205462948
 */






void  decToBinary(int n , char &c) 
{ 
   // array to store binary number 
    int binaryNum[8]; 
  
    // counter for binary array 
    int i = 0; 
    while (n > 0) { 
          // storing remainder in binary array 
        binaryNum[i] = n % 2; 
        n = n / 2; 
        i++; 
    } 
  
    // printing binary array in reverse order 
    for (int j = i - 1; j >= 0; j--) {
        if (binaryNum[j]==1)
            c = c | 1u << j;
    }
 } 

// #define SYS_CALL
// ============================================================================
class fsInode {
    int fileSize;
    int block_in_use;

    int *directBlocks;
    int singleInDirect;
    int num_of_direct_blocks;
    int block_size;
    


    public:
    fsInode(int _block_size, int _num_of_direct_blocks) {
        fileSize = 0; 
        block_in_use = 0; 
        block_size = _block_size;
        num_of_direct_blocks = _num_of_direct_blocks;
        directBlocks = new int[num_of_direct_blocks];
		    assert(directBlocks);
        for (int i = 0 ; i < num_of_direct_blocks; i++) {   
            directBlocks[i] = -1;
        }
        singleInDirect = -1;
        
    }


	// its help mathods to set and get values
    int getSingleInDirect(){
        return singleInDirect;
    }
    void setSingleInDirect(int x){
        singleInDirect = x;
    }
    int* getDirectBlocks(){
        return directBlocks;
    }
    void setFileSize(int x){
        fileSize += x;
    //   printf("the lenght is %d\n", fileSize);

    }
    void setBluckInUse(int x){
        block_in_use += x;
    }
    int getFileSize(){
        // printf("the lenght is  IS: %d\n", fileSize);

        return fileSize;
    }
    int getBlockInUse(){
        return block_in_use;
    }
    ~fsInode() { 
        delete directBlocks;
    }


};

// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

    public:
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName;
        file.second = fsi;
        inUse = true;

    }

    string getFileName() {
        return file.first;
    }

    fsInode* getInode() {
        

        return file.second;

    }
//in case that we delete a file we make all the fileds to be empty

    void setInodeToNull (){
        file.first = "";
        file.second = NULL;
        

    }
    bool isInUse() { 
        return (inUse); 
    }
    void setInUse(bool _inUse) {
        inUse = _inUse ;
    }
    
    // ~FileDescriptor(){
        
    // }
};
 
#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE *sim_disk_fd;
 
    bool is_formated;

	// BitVector - "bit" (int) vector, indicate which block in the disk is free
	//              or not.  (i.e. if BitVector[0] == 1 , means that the 
	//             first block is occupied. 
    int BitVectorSize;
    int *BitVector;

    // Unix directories are lists of association structures, 
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir ; 

    // OpenFileDescriptors --  when you open a file, 
	// the operating system creates an entry to represent that file
    // This entry number is the file descriptor. 
    vector< FileDescriptor > OpenFileDescriptors;

    int direct_enteris;
    int block_size;
    int capacity_of_block_left;

    public:
    // ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen( DISK_SIM_FILE , "r+" );
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
        
    }
	


    // ------------------------------------------------------------------------
    void listAll() {
        int i = 0;    
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->getFileName() <<  " , isInUse: " << it->isInUse() << endl; 
            i++;
        }
        char bufy;
        cout << "Disk content: '" ;
        for (i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
             cout << bufy;              
        }
        cout << "'" << endl;


    }
 
    // ------------------------------------------------------------------------
    void fsFormat( int blockSize =4, int direct_Enteris_ = 3 ) {
        // update all the values and init' 
        this->direct_enteris = direct_Enteris_;
        this->block_size = blockSize;
        this->BitVectorSize = DISK_SIZE / block_size;
        this->BitVector = new int [BitVectorSize];
       //put in all the bit vactor zeros - means all the blocks are free
        for (int i = 0; i < this->BitVectorSize; i++)
        {
        this->BitVector [i] = 0;
        }
        capacity_of_block_left = DISK_SIZE / block_size;
        is_formated = true;

        cout << "format Disk: number of blocks: " << BitVectorSize<< "\n";
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
      		//check that the file name isn't already exist
        for (int i = 0; i < OpenFileDescriptors.size(); i++)
        {
            if ((OpenFileDescriptors[i].getFileName().compare(fileName))==0)
            {
                printf("this name already exist\n");
                return -1;
            }
        }
        
        //create a new file and update all the right places
        fsInode *fs1 = new fsInode(block_size , direct_enteris);
        FileDescriptor *fd1 = new FileDescriptor(fileName, fs1);
        // this->MainDir[fileName] = fs1;
        MainDir.insert(pair<string , fsInode*>(fileName , fs1));
        int temp_size = this->OpenFileDescriptors.size();
        this->OpenFileDescriptors.push_back(*fd1);
        
        if(is_formated)
    //    return the fd of the file that created
        return temp_size;
        return -1;
        
    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName) {
        
        bool flag = false;
        int toReturn = -1;
        //check if the file in open file descriptor and if its open
        for (int i = 0; i < OpenFileDescriptors.size(); i++)
        {
            if ((OpenFileDescriptors[i].getFileName().compare(fileName))==0)
            {
                flag = true;
                if (OpenFileDescriptors[i].isInUse())
                {
                    printf("file alredy open\n");
                    return i;
                }
                else
                {
                    OpenFileDescriptors[i].setInUse(true);
                    return i;
                }
            }
            
        }
        // in case the file is not in the file descriptor we bring it form main dir
        if (!flag)
        {
           printf("file does not exist\n");
            return -1;
        ///in case we need to delete from open file description
        //    fsInode *fs1 = MainDir[fileName];
        //    FileDescriptor *fd1=new FileDescriptor(fileName, fs1);
        //    fd1->setInUse(true);
        //    toReturn = OpenFileDescriptors.size();
        //    OpenFileDescriptors.push_back(*fd1);
        }
        return toReturn;
        
    }  

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        //check if the fd in the right range
       int size = OpenFileDescriptors.size();
        if (fd > size)
        {
            printf("file does not exist\n");
            return "-1";
        }
        //check if its already close
        if (!OpenFileDescriptors[fd].isInUse())
        {
            printf("file are not open\n");
            return "-1";   
        }
        		//close the file
        OpenFileDescriptors[fd].setInUse(false);
        /////////////////?????????????????? do i need to erase the file from open file descriptor???
        // OpenFileDescriptors.erase(fd);
        
        return OpenFileDescriptors[fd].getFileName();
        
    }
    // ------------------------------------------------------------------------
//alocate a free block to put data on him

    int findEmptyBlock(){

        for (int i = 0; i < BitVectorSize; i++)
        {
            if (BitVector[i]==0)
            {
                BitVector[i] = 1;
                return i;
            } 
        }
        printf("there is no empty block\n");
        return -1;
    }
    
   
    int WriteToFile(int fd, char *buf, int len ) {
        //check if the system is formated

        if (!is_formated)
        {
            printf("the system is not formated\n");
            return -1;
        }
        //check if we open the file
        if (!OpenFileDescriptors[fd].isInUse())
        {
            printf("the file is not opened \n");
            return -1;
        }
        //check if there any space left in the last block that we used
        fsInode *fs1 = OpenFileDescriptors[fd].getInode();
        int file_size = fs1->getFileSize();
        int room_in_last_block = block_size - (file_size % block_size);
        if(room_in_last_block == block_size) room_in_last_block = 0;
        int number_of_block_needed = (len  - room_in_last_block)/ block_size;
        if (len % block_size != 0)
        {
            number_of_block_needed++;
        }
        
        
        //check if there is enough space on disk
        if (number_of_block_needed > capacity_of_block_left)
        {
            printf("there is not enough space in disk \n");
            return -1;
        }
        //check if there is enough space on file:
        int max_Inode_size = (direct_enteris + block_size) * block_size;
        if (len + file_size > max_Inode_size)
        {
            printf("there is not enough space in file \n");
            return -1;   
        }
        fs1->setFileSize(len);
        //we write to the disk 
        //first we try to wtire to direct :
        int right_place = 0 , i = 0;
        int block_in_use = fs1->getBlockInUse();
        if (block_in_use < direct_enteris)
        {
            int * direct_blocks = fs1-> getDirectBlocks();
            //fill a block if there is a block that not full
            if (room_in_last_block != 0)
            {
                //find the right place to write
                right_place = direct_blocks[block_in_use - 1] * block_size +room_in_last_block;
                
            	//put a pointer to the right place on the disk
                int ret_val = fseek ( sim_disk_fd , right_place , SEEK_SET );
                ret_val = fwrite( buf ,  sizeof(char) , room_in_last_block , sim_disk_fd );
                i += room_in_last_block ;

                room_in_last_block = 0;
                if(i >= len){
                    fflush(sim_disk_fd);
                    return 1;
                }
            }


            //fill all the other block in direct
            int num_of_blocks_to_fill_in_direct =number_of_block_needed - (number_of_block_needed - (direct_enteris - block_in_use));
            
            if(num_of_blocks_to_fill_in_direct <= 0) num_of_blocks_to_fill_in_direct = number_of_block_needed;

            for (int j = 0; j < num_of_blocks_to_fill_in_direct; j++)
            {


                direct_blocks[block_in_use + j] =   findEmptyBlock();


                fs1->setBluckInUse(1);
                capacity_of_block_left --;
                right_place = direct_blocks[block_in_use + j] *block_size;
                for (int k = 0; k < block_size; k++)
                {
                    int ret_val = fseek ( sim_disk_fd , right_place +k, SEEK_SET );
                    ret_val = fwrite( &buf[i] ,  sizeof(char) , 1 , sim_disk_fd );
                    // printf("char i: %s \n", &buf[i]);
                    i++;
                }
                //if we finish writing
                if(i >= len){
                    fflush(sim_disk_fd);
                    return 1;
                }
                
            }
               //if we finish writing
                if(i >= len){
                    fflush(sim_disk_fd);
                    return 1;
                }
            
        }
        
        //if the storge on direct end: we go to single
        int SingleInDirect = fs1->getSingleInDirect();
        if (SingleInDirect != -1)
        {
            if (room_in_last_block != 0)
            {
                char bufy;
                int which_block_on_single = (file_size - (direct_enteris*block_size))/block_size;
                //----need to check the one
                right_place = SingleInDirect*block_size + which_block_on_single +1;
                int temp_right_place = right_place;
                int ret_val = fseek ( sim_disk_fd , right_place , SEEK_SET );
                ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
                int num_of_block = (int) bufy ;
                right_place = num_of_block *block_size;

                for (int j = 0; j < room_in_last_block; j++)
                {
                   int ret_val = fseek ( sim_disk_fd , right_place +j, SEEK_SET );
                   ret_val = fwrite( &buf[i] ,  sizeof(char) , 1 , sim_disk_fd );
                   i++;
                    
                }
                // keep writeing to the other blocks
                right_place = temp_right_place;
                int num_of_block_needed_in_disk = number_of_block_needed -(direct_enteris -block_in_use);
                for (int j = 0; j < num_of_block_needed_in_disk; j++)
                {
                    int ret_val = fseek ( sim_disk_fd , right_place+j , SEEK_SET );
                    char c = '0';
                    int temp_num_of_block = findEmptyBlock();
                    capacity_of_block_left --;
                    fs1->setBluckInUse(1);
                    decToBinary(temp_num_of_block, c);
                    ret_val = fwrite( &c ,  sizeof(char) , 1 , sim_disk_fd );
                    for (int k = 0; k < block_size; k++)
                    {
                        right_place =  temp_num_of_block * block_size;
                        int ret_val = fseek ( sim_disk_fd , right_place +k , SEEK_SET );
                        ret_val = fwrite( &buf[i] ,  sizeof(char) , 1 , sim_disk_fd );
                        i++;
                        //if we finish writing
                        if(i >= len){
                            fflush(sim_disk_fd);
                            return 1;
                        }

                    }
                    //if we finish writing
                    if(i >= len){
                        fflush(sim_disk_fd);
                        return 1;
                    }
                    
                }

                
                

            }            
            //if we finish writing
                if(i >= len){
                    fflush(sim_disk_fd);
                    return 1;
                }
        }
        //if we haven't put anything yet in single
        if (SingleInDirect == -1)
        {
            // printf("IM HERE \n");
        	//put in single a number of block - to put in it numbers of blocks
            SingleInDirect = findEmptyBlock();
            fs1->setSingleInDirect(SingleInDirect);
            capacity_of_block_left--;
            right_place = SingleInDirect * block_size; 
            int ret_val = fseek ( sim_disk_fd , right_place , SEEK_SET );
            ret_val = fwrite( "00000" ,  sizeof(char) , 5 , sim_disk_fd );
            int num_of_block_needed_in_disk = number_of_block_needed -(direct_enteris -block_in_use);
    		//write to the disk the needed data
            for (int j = 0; j < num_of_block_needed_in_disk; j++)
            {
                char c = '0';
                int temp_num_of_block = findEmptyBlock();
                capacity_of_block_left --;
                fs1->setBluckInUse(1);
                decToBinary(temp_num_of_block, c);
                right_place = SingleInDirect * block_size; 
                int ret_val = fseek ( sim_disk_fd , right_place+j , SEEK_SET );
                ret_val = fwrite( &c ,  sizeof(char) , 1 , sim_disk_fd );
                for (int k = 0; k < block_size; k++)
                {
                    right_place =  temp_num_of_block * block_size;
                    int ret_val = fseek ( sim_disk_fd , right_place +k , SEEK_SET );
                    ret_val = fwrite( &buf[i] ,  sizeof(char) , 1 , sim_disk_fd );
                    i++;
                    //if we finish writing
                    if(i >= len){
                        fflush(sim_disk_fd);
                        return 1;
                    }
                }
                    //if we finish writing
                    if(i >= len){
                        fflush(sim_disk_fd);
                        return 1;
                    }                
            }
            
        }
    
    fflush(sim_disk_fd);

    }


/*
    // ------------------------------------------------------------------------
    int WriteToFile1(int fd, char *buf, int len ) {
        int left_to_write =len;
        //check if there is enough free spece om file
        int max_Inode_size = (direct_enteris + block_size) * block_size;
        if (fd > OpenFileDescriptors.size())
        {
            printf("fd not in bound\b");
            return -1;
        }
        fsInode *fs1 = OpenFileDescriptors[fd].getInode();
        // fsInode *fs1 = OpenFileDescriptors.at(fd).getInode();
        
        int file_size = fs1->getFileSize();
        if (file_size + len >= max_Inode_size  )
        {
            printf("there is not enough space in file\n ");
            return -1;
        }else {
            // //check if there is enough free blocks on disk

            int room_in_last_block = block_size - (file_size % block_size);
            if (room_in_last_block == block_size)
            {
                room_in_last_block = 0;
            }
            //check how many blocks needed to store the data
            int number_of_block_needed = (len  - room_in_last_block)/ block_size;
            if ((len - room_in_last_block) % block_size != 0)
                {
                    number_of_block_needed++;
                }
            
            int num_of_block_in_inode = max_Inode_size / block_size;

            int num_of_block_needed_in_disk = number_of_block_needed - (num_of_block_in_inode - fs1->getBlockInUse());

            if (num_of_block_needed_in_disk >= 0 && capacity_of_block_left - num_of_block_needed_in_disk<0)
                {
                    printf("there is not enough space in storge-disk\n");
                    return -1;
                }
            
            //check if the system is formated
            if (!is_formated)
            {
                printf("the system is not formated\n");
                return -1;
            }
            //check if we open the file
            if (!OpenFileDescriptors[fd].isInUse())
            {
                printf("the file is not opened \n");
                return -1;
            }
            //find the right place
            int right_placed = 0 ;
            int free_in_direct = num_of_block_in_inode - fs1->getBlockInUse();
            fs1->setFileSize(len);
            //if there is free block in direct we fill it first
            if (free_in_direct > 0)
            {   
                int *DirectBlocks  = fs1->getDirectBlocks();
                //write to the block with room in it
                if (room_in_last_block!=0)
                {
                int block_with_room = file_size / block_size;            
                right_placed =  (DirectBlocks[block_with_room] * block_size) + (block_size- room_in_last_block + 1);
                int ret_val = fseek ( sim_disk_fd , right_placed , SEEK_SET );
                //need to check the buf
                ret_val = fwrite( buf ,  sizeof(char) , room_in_last_block , sim_disk_fd );
                assert(ret_val == 1);
                left_to_write -=room_in_last_block;
                // cout << "im here 0\n";
                }


                // fill the blocks by order
                for (int i = 0; i < number_of_block_needed-num_of_block_needed_in_disk && left_to_write>0 ; i++)
                {
                    

                    DirectBlocks[i] = findEmptyBlock();

                    int temp_num_of_block = DirectBlocks[i];
                    
                    right_placed = temp_num_of_block * block_size;

                    int ret_val = fseek ( sim_disk_fd , right_placed , SEEK_SET );
                    //need to check the buf
                    ret_val = fwrite( buf ,  1 , block_size , sim_disk_fd );
                    // assert(ret_val == 1);
                    left_to_write -=block_size;
                    fs1->setBluckInUse(1);
                    capacity_of_block_left--;
                    cout << "im here 1\n";

                }
            }
            //if there not enough space in direct we write into single
            if (num_of_block_needed_in_disk>0)
            {   

                int SingleInDirect = fs1->getSingleInDirect();
                if (SingleInDirect == -1)
                {
                    //find an empty block on disk
                    for (int i = 0; i < BitVectorSize; i++)
                    {
                        if (BitVector[i] == 0)
                        {
                            SingleInDirect = i;
                            fs1->setSingleInDirect(i);
                            BitVector[i] = 1;
                            capacity_of_block_left--;
                            right_placed = SingleInDirect * block_size; 
                            int ret_val = fseek ( sim_disk_fd , right_placed , SEEK_SET );
                            ret_val = fwrite( "00000" ,  sizeof(char) , 5 , sim_disk_fd );
                            assert(ret_val == 1);

                            break;
                        }
                    }

                }

                
                //write on single block
                for (int i = 0; i < num_of_block_needed_in_disk && left_to_write>0 ; i++)
                {
                    int temp_block=0;
                    for (int j = 0; j < BitVectorSize; i++)
                    {
                        if (BitVector[j] == 0)
                        {
                            //coding the int to be char to fit in a single char in disk
                            char c;
                            decToBinary(j, c);
                            right_placed = SingleInDirect * block_size;
                            right_placed += i;
                            int ret_val = fseek ( sim_disk_fd , right_placed , SEEK_SET );
                            assert(ret_val == 1);
                            BitVector[j] = 1;
                            temp_block = j;
                            ret_val = fwrite( &c ,  sizeof(char) , 1 , sim_disk_fd );
                            break;
                        }
                        
                    }
                    right_placed = temp_block * block_size; 
                    int ret_val = fseek ( sim_disk_fd , right_placed , SEEK_SET );
                    //need to check the buf
                    ret_val = fwrite( buf ,  sizeof(char) , block_size , sim_disk_fd );
                    assert(ret_val == 1);
                    left_to_write -=block_size;
                    fs1->setBluckInUse(1);
                    capacity_of_block_left--;
            
                }
                
            }
            
        }

        
        
    }
  
  */
    // ------------------------------------------------------------------------
    int DelFile( string FileName ) {

    	//find the file and update his filed
        for (int i = 0; i < OpenFileDescriptors.size(); i++)
        {
            if ((OpenFileDescriptors[i].getFileName().compare(FileName))==0){
                OpenFileDescriptors[i].setInUse(false);
                delete OpenFileDescriptors[i].getInode();
                // OpenFileDescriptors[i].setInodeToNull();
                
                break;
            }
        }
        fsInode *fs1 = MainDir[FileName];
        int block_in_use = fs1->getBlockInUse();
        int *direct_block = fs1->getDirectBlocks();
        int single_block = fs1-> getSingleInDirect();
        char bufy;
        int num_of_block = 0;
        int right_place = 0;
    	//clear all the block the used
        for (int i = 0, j=0; i < block_in_use; i++)
        {
            if (block_in_use <  direct_enteris )
            {
                BitVector[direct_block[i]] = 0;
            }else
            {   
                right_place = single_block * block_size;
                int ret_val = fseek ( sim_disk_fd , right_place +j , SEEK_SET );
                ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
                num_of_block =(int)bufy;
                BitVector[num_of_block] = 0;
                j++;
            }
            
            
        }
        
        
        delete fs1;
        MainDir.erase(FileName);
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len ) { 
    	//check if the file exist
      if (fd > OpenFileDescriptors.size())
      {
          printf("fd out of bound \n");
          return -1;
      }
      //check if the file already closed
      if (!OpenFileDescriptors[fd].isInUse())
      {
          printf("the file is closed\n");
          return -1;
      }
      fsInode *fs1 = OpenFileDescriptors[fd].getInode();
      //check if you didn't ask for too long lenght
      if (fs1->getFileSize() < len)
      {
        //   printf("the lenght is %d %d\n",len, fs1->getFileSize());
          printf("you requsted length that longer from the file\n");
          return -1;
      }
      char bufy;
      int left_to_read = len ;
      int right_place = 0;
      int * direct_blocks = fs1->getDirectBlocks();
      
      int i =0;
    //    printf("the NUM OF direct_enteris is %d \n", direct_enteris);
    //    printf("the left_to_read%d \n", left_to_read);
        //start to read from the right place
		// read from direct blocks
      while (len > 0  && i < direct_enteris)
      {

        right_place = direct_blocks [i] *block_size;
        fseek ( sim_disk_fd , right_place , SEEK_SET );
        fread(  &bufy , 1 , block_size, sim_disk_fd );
        
        strcat(buf,&bufy);  
        //left to read -just becuse without it read change i
        left_to_read -= block_size;
        len-= block_size;
            //read one carcter each time
            // right_place = direct_blocks [i] *block_size;
            // for (int j = 0; j < block_size && len > 0; j++)
            // {
            //      int ret_val = fseek ( sim_disk_fd , right_place+j , SEEK_SET );
            //     ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
            //     // left_to_read -= block_size;
            //     strcat(buf,&bufy);  
            //     // left_to_read -= block_size;
            //     len--;
            //     printf("im here0 %d %d\n", len, i);
            // }


        i++;
        
        

      }
        
        
        i = 0;
        int num_of_block = 0;
        int single_block = fs1-> getSingleInDirect();
        //read from signal blocks
        while (len > 0)
        {
            right_place = single_block * block_size;
            int ret_val = fseek ( sim_disk_fd , right_place +i , SEEK_SET );
            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
            num_of_block =(int)bufy - 48;
            // printf("imt here 2 %d\n",num_of_block);
            
            right_place = num_of_block * block_size;
            ret_val = fseek ( sim_disk_fd , right_place  , SEEK_SET );
            ret_val = fread(  &bufy , 1 , block_size, sim_disk_fd );
            // printf("%s\n" , &bufy);
            strcat(buf,&bufy);  
            left_to_read -= block_size;
            len -= block_size;

            i++;
        }
        
	    fflush(sim_disk_fd);
  
    }
    ~fsDisk(){
      fclose(sim_disk_fd); 
      delete (BitVector);
      for (int i = 0; i < OpenFileDescriptors.size(); i++)
      {
          delete (OpenFileDescriptors[i].getInode());
          FileDescriptor *fd1 =(FileDescriptor*) &OpenFileDescriptors.front();
            
          delete (fd1);
      }
      

    }

};
    
int main() {
    int blockSize; 
	int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read; 
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
				delete fs;
				exit(0);
                break;

            case 1:  // list-file
                fs->listAll(); 
                break;
          
            case 2:    // format
                cin >> blockSize;
				cin >> direct_entries;
                fs->fsFormat(blockSize, direct_entries);
                break;
          
            case 3:    // create-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
            
            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
             
            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd); 
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
           
            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;
          
            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;

                memset(str_to_read, 0, size_to_read);

                break;
           
            case 8:   // delete file 
                 cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
            default:
                break;
        }
    }

} 