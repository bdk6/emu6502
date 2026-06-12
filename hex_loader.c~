/// @file hex_loader.c
/// @brief loads intel hex file records of format 00, 01
/// @copyright 2025 William R Cooke



#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

FILE* hf;

#define MAX_RECORD_LENGTH 255

typedef enum
  {
    HEX_ERR_NONE    = 0,
    HEX_ERR_NOFILE  = (1<<0),
    HEX_ERR_EOF     = (1<<1),
    HEX_ERR_CS      = (1<<2),
    HEX_ERR_TYPE    = (1<<3),
    HEX_ERR_FORMAT  = (1<<4)
  } hex_error_t;

typedef struct
{
  hex_error_t error;
  int rec_type;
  int address;
  int bytes;
  uint8_t data[MAX_RECORD_LENGTH];
} hex_record_t;

  

int hex_open(char* path)
{
  int rtn = 0;


  return rtn;
}

int hex_close(void)
{
  int rtn = 0;

  return rtn;
}

static int hexval(int ch)
{
  int rtn = 0;
  ch = toupper(ch);
  if(ch >= '0' && ch <= '9')
    {
      rtn = ch - '0';
    }
  else if(ch >= 'A' && ch <= 'F')
    {
      rtn = ch - 'A' + 10;
    }
  else
    {
      rtn = -1;
    }

  return rtn;
}

int byteval(char c1, char c2)
{
  int rtn = 0;
  c1 = hexval(c1);
  c2 = hexval(c2);
  if(c1 < 0 || c2 < 0)
    {
      rtn = -1;
    }
  else
    {
      rtn = c1 * 16 + c2;
    }


  return rtn;
}


int hex_read_record(hex_record_t * rec)
{
  int rtn = 0;
  uint16_t address = 0;
  
  if(rec == NULL)
    {
      rtn = -1;
      rec->error |= HEX_ERR_NOFILE;
    }
  else
    {
      rec->error = HEX_ERR_NONE;
      rec->rec_type = 0;
      rec->address = 0;
      rec->bytes = 0;
      
      int ch1;
      int ch2;
      int byte;
      int checksum = 0;
      int bytes = 0;
      
      // skip leading characters
      do
	{
	  ch1 = getc(hf);
	} while(ch1 != ':' && ch1 != -1);
      if(ch1 == -1) // eof
	{
	  rtn = -1;
	  rec->error = HEX_ERR_EOF;
	}
      else // colon -- start of record
	{
	  // data byte count
	  ch1 = getc(hf);
	  ch2 = getc(hf);
	  byte = byteval(ch1, ch2);
	  if(byte >= 0)
	    {
	      if(byte < 0)
		{
		  rec->error |= HEX_ERR_FORMAT;
		  rtn = -1;
		}
	      checksum += byte;
	      bytes = byte;
	      rec->bytes = byte;

	      // address
	      ch1 = getc(hf);
	      ch2 = getc(hf);
	      byte = byteval(ch1, ch2);
	      if(byte < 0)
		{
		  rec->error |= HEX_ERR_FORMAT;
		  rtn = -1;
		}
	      checksum += byte;
	      address = byte << 8;
	      ch1 = getc(hf);
	      ch2 = getc(hf);
	      byte = byteval(ch1, ch2);
	      if(byte < 0)
		{
		  rec->error |= HEX_ERR_FORMAT;
		  rtn = -1;
		}
	      checksum += byte;
	      address += byte;
	      rec->address = address;

	      // type
	      ch1 = getc(hf);
	      ch2 = getc(hf);
	      byte = byteval(ch1, ch2);
	      if(byte < 0)
		{
		  rec->error |= HEX_ERR_FORMAT;
		  rtn = -1;
		}
	      checksum += byte;
	      rec-> rec_type = byte;
	      // only support types 0 and 1
	      if(byte != 0 && byte != 1)
		{
		  rec->error |= HEX_ERR_TYPE;
		  rtn = -1;
		}

	      // data
	      for(int i = 0; i < bytes; i++)
		{
		  ch1 = getc(hf);
		  ch2 = getc(hf);
		  byte = byteval(ch1, ch2);
		  checksum += byte;
		  rec->data[i] = byte;
		}
	      // checksum
	      ch1 = getc(hf);
	      ch2 = getc(hf);
	      byte = byteval(ch1, ch2);
	      if( ((byte + checksum) & 0xff) != 0)
		{
		  rec-> error |= HEX_ERR_CS;
		  rtn = -1;
		}
	    }
	  else
	    {
	      rec->error |= HEX_ERR_FORMAT;
	    }

	  // If no error, return number of bytes read
	  if(rtn == 0)
	    {
	      rtn = rec->bytes;
	    }
	}  

    }
  
  return rtn;
}


  int main(int argc, char* argv[])
  {


    return 0;
  }
  
