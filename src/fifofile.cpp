#include "fifofile.h"
#include <iostream>

FIFOIO::~FIFOIO()
{
    closeFIFOFile();
}


FIFOIO::FIFOIO()
{

}


///
/// \brief fullpath2Path C:\Test\abc.xyz --> C:\Test
/// \param strFullName
/// \return path without short name
///
std::string fullpath2Path(std::string strFullName)
{
    if (strFullName.empty())
    {
        return "";
    }

    //string_replace(strFullName, "/", "\\");

    std::string::size_type iPos = strFullName.find_last_of('/') + 1;

    return strFullName.substr(0, iPos-1);
}

FIFOIO::FIFOIO(const char* p_fifo_name, int mode )
{
    this->setFIFOName(p_fifo_name);
    this->openFIFOFile(mode);
}


void FIFOIO::setFIFOName(const char* p_fifo_name)
{
    if(NULL == p_fifo_name)
        throw "Invalid argument(s)";
    strcpy(this->fifo_name, p_fifo_name);
    std::string path = fullpath2Path(fifo_name);
    if(access(path.c_str(), 0) == -1) //access函数是查看文件是不是存在
    {
        std::cout<<"create file folder: "<<path<<std::endl;
        if (mkdir(path.c_str(), 0777)) //如果不存在就用mkdir函数来创建
        {
            perror("create file folder failed");
        }
    }
}


/*
    设置数据尺寸：长、宽、每个元素的字节数
    */
void FIFOIO::setDataSize(unsigned int width, unsigned int height, size_t bytes_per_element)
{
    this->data_width = width;
    this->data_height = height;
    //this->bytes_per_element = bytes_per_element;
    this->nbytes = width * height * bytes_per_element;
    this->buffer = (char*)malloc(this->nbytes);
}

/*
    @brief 打开管道文件
    @param mode: 0-read, 1-write
*/
void FIFOIO::openFIFOFile(int mode)
{
    printf("fifoname: %s\n", fifo_name);
    int res = 0;
 
    if(access(fifo_name, F_OK) == -1)
    {
        printf ("Create the fifo pipe.\n");
        res = mkfifo(fifo_name, 0644);
 
        if(res != 0)
        {
            fprintf(stderr, "[%s line:%d] Could not create fifo %s\n", __FILE__, __LINE__, fifo_name);
            exit(EXIT_FAILURE);
        }
    }

    int open_mode;
    if(mode == 0)//read
    {
        open_mode = O_RDONLY;
        printf("Process %d opening FIFO O_RDONLY\n", getpid());
    }
    else if(mode == 1)//write
    {
        open_mode = O_WRONLY;
        printf("Process %d opening FIFO O_WRONLY\n", getpid());
    }
    else
    {
        fprintf(stderr, "[%s line:%d] did not set open mode.\n", __FILE__, __LINE__);
        throw mode;
    }
    
    this->pipe_fd = open(fifo_name, open_mode);
    printf("Process %d result %d\n", getpid(), this->pipe_fd);
}

void FIFOIO::closeFIFOFile()
{
    close(this->pipe_fd);
    printf("Process %d finished\n", getpid());
    exit(EXIT_SUCCESS);
}

/*
    @brief 压入管道数据
    @param data 输入数据
    @return 压入数据bytes
*/
size_t FIFOIO::push_back(unsigned char* data)
{
    if(this->pipe_fd == -1)
    {
        //exit(EXIT_FAILURE);
        fprintf(stderr, "[%s line:%d] fifo is not opening\n", __FILE__, __LINE__);
        //return 0;
        throw this->pipe_fd;//throw尽量不需要用户判断返回值，无法忽视异常
    }
    
    if(NULL == data)
    {
        close(this->pipe_fd);
        fprintf(stderr, "[%s line:%d] pushback data failed\n", __FILE__, __LINE__);
        //return 0;
        throw data;
    }
    int res = write(this->pipe_fd, data, this->nbytes);
    if(res == -1)
    {
        fprintf(stderr, "[%s line:%d] Write error on pipe\n", __FILE__, __LINE__);
        //return 0;
        throw res;
        //exit(EXIT_FAILURE);
    }
    return res;

}


/*
    弹出管道数据，先进先出
*/
char* FIFOIO::pop_front()
{
    if(this->pipe_fd == -1)
    {
        fprintf(stderr, "[%s line:%d] fifo is not opening\n", __FILE__, __LINE__);
        throw this->pipe_fd;
    }
    
    if (this->nbytes == 0)
    {
        fprintf(stderr, "[%s line:%d] size of data is not set.\n", __FILE__, __LINE__);
        throw this->nbytes;
    }

    int res = read(this->pipe_fd, this->buffer, this->nbytes);
    this->rec_bytes = res;
    if(res == -1)
    {
        fprintf(stderr, "[%s line:%d] read error on pipe\n", __FILE__, __LINE__);
        throw res;
    }

    return this->buffer;
}

int FIFOIO::writeFIFOTest()
{
    printf("fifoname: %s\n", fifo_name);
    int pipe_fd = -1;
    int data_fd = -1;
    int res = 0;
    const int open_mode = O_WRONLY;
    int bytes_sent = 0;
    char buffer[PIPE_BUF + 1];
    int bytes_read = 0;
 
    if(access(fifo_name, F_OK) == -1)
    {
        printf ("Create the fifo pipe.\n");
        res = mkfifo(fifo_name, 0644);
 
        if(res != 0)
        {
            fprintf(stderr, "[%s line:%d] Could not create fifo %s\n", __FILE__, __LINE__, fifo_name);
            exit(EXIT_FAILURE);
        }
    }
 
    printf("Process %d opening FIFO O_WRONLY\n", getpid());
    pipe_fd = open(fifo_name, open_mode);
    printf("Process %d result %d\n", getpid(), pipe_fd);
	//printf("%d\n",pipe_fd);
 
    if(pipe_fd != -1)
    {
        while(1)
        {
            bytes_read = 0;
            data_fd = open("/home/jc-acc/pipeline_file_queue/tmp/Data.txt", O_RDONLY);
            if (data_fd == -1)
            {
                close(pipe_fd);
                fprintf (stderr, "[%s line:%d] Open file[Data.txt] failed\n", __FILE__, __LINE__);
                return -1;
            }
 
            bytes_read = read(data_fd, buffer, PIPE_BUF);////一次读完全部内容
            buffer[bytes_read] = '\0';
            while(bytes_read > 0)
            {
                res = write(pipe_fd, buffer, bytes_read);
                if(res == -1)
                {
                    fprintf(stderr, "[%s line:%d] Write error on pipe\n", __FILE__, __LINE__);
                    exit(EXIT_FAILURE);
                }
 
                bytes_sent += res;
                bytes_read = read(data_fd, buffer, PIPE_BUF);
                buffer[bytes_read] = '\0';
            }
            close(data_fd);

            usleep(20*1000);//20ms
        }
        close(pipe_fd);
        
    }
    else
        exit(EXIT_FAILURE);
 
    printf("Process %d finished\n", getpid());
    exit(EXIT_SUCCESS);
}


int FIFOIO::readFIFOTest()
{
    int pipe_fd = -1;
    int data_fd = -1;
    int res = 0;
    int open_mode = O_RDONLY;
    char buffer[PIPE_BUF + 1];
    int bytes_read = 0;
    int bytes_write = 0;
 
    memset(buffer, '\0', sizeof(buffer));
 
    printf("Process %d opening FIFO O_RDONLY\n", getpid());
    pipe_fd = open(fifo_name, open_mode);
    data_fd = open("/home/jc-acc/pipeline_file_queue/tmp/DataFromFIFO.txt", O_WRONLY|O_CREAT, 0644);
    
 
    if (data_fd == -1)
    {
        fprintf(stderr, "[%s line:%d] Open file[DataFromFIFO.txt] failed\n", __FILE__, __LINE__);
        close(pipe_fd);
        return -1;
    }
 
    printf("Process %d result %d\n",getpid(), pipe_fd);
    if(pipe_fd != -1)
    {
        do
        {
            res = read(pipe_fd, buffer, PIPE_BUF);//一次读完全部内容
            bytes_write = write(data_fd, buffer, res);
            //printf("%d\n",bytes_read);
            bytes_read += res;
        }while(res > 0);

        close(data_fd);
        close(pipe_fd);
        
    }
    else
        exit(EXIT_FAILURE);
 
     printf("Process %d finished, %d bytes read\n", getpid(), bytes_read);
     exit(EXIT_SUCCESS);
}



//reference
//[1] https://blog.csdn.net/tiramisu_L/article/details/80176350