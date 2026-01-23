#include "File_Handling_RTOS.h"
#include "main.h"

/* =============================>>>>>>>> NO CHANGES AFTER THIS LINE =====================================>>>>>>> */

FATFS fs;  // file system
FIL fil; // File
FILINFO fno;
FRESULT fresult;  // result
UINT br, bw;  // File read/write count

/**** capacity related *****/
FATFS *pfs;
DWORD fre_clust;
uint32_t total, free_space;

void Mount_SD (const TCHAR* path)
{
	fresult = f_mount(&fs, path, 1);
	if (fresult != FR_OK) Print_Msg ("ERROR!!! in mounting SD CARD...\r\n\n");
	else Print_Msg("SD CARD mounted successfully...\r\n");
}

void Unmount_SD (const TCHAR* path)
{
	fresult = f_mount(NULL, path, 1);
	if (fresult == FR_OK) Print_Msg ("SD CARD UNMOUNTED successfully...\r\n\n\n");
	else Print_Msg("ERROR!!! in UNMOUNTING SD CARD\r\n\n\n");
}

/* Start node to be scanned (***also used as work area***) */
FRESULT Scan_SD (char* pat)
{
    DIR dir;
    UINT i;
    char *path = pvPortMalloc(20*sizeof (char));
    sprintf (path, "%s",pat);

    fresult = f_opendir(&dir, path);                       /* Open the directory */
    if (fresult == FR_OK)
    {
        for (;;)
        {
            fresult = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (fresult != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR)     /* It is a directory */
            {
            	if (!(strcmp ("SYSTEM~1", fno.fname))) continue;
            	char *buf = pvPortMalloc(30*sizeof(char));
            	sprintf (buf, "Dir: %s\r\n", fno.fname);
            	Print_Msg(buf);
            	vPortFree(buf);
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                fresult = Scan_SD(path);                     /* Enter the directory */
                if (fresult != FR_OK) break;
                path[i] = 0;
            }
            else
            {   /* It is a file. */
           	   char *buf = pvPortMalloc(30*sizeof(char));
               sprintf(buf,"File: %s/%s\r\n", path, fno.fname);
               Print_Msg(buf);
               vPortFree(buf);
            }
        }
        f_closedir(&dir);
    }
    vPortFree(path);
    return fresult;
}

/* Only supports removing files from home directory */
FRESULT Format_SD (void)
{
    DIR dir;
    char *path = pvPortMalloc(20*sizeof (char));
    sprintf (path, "%s","/");

    fresult = f_opendir(&dir, path);                       /* Open the directory */
    if (fresult == FR_OK)
    {
        for (;;)
        {
            fresult = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (fresult != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR)     /* It is a directory */
            {
            	if (!(strcmp ("SYSTEM~1", fno.fname))) continue;
            	fresult = f_unlink(fno.fname);
            	if (fresult == FR_DENIED) continue;
            }
            else
            {   /* It is a file. */
               fresult = f_unlink(fno.fname);
            }
        }
        f_closedir(&dir);
    }
    vPortFree(path);
    return fresult;
}




FRESULT Write_File (char *name, char *data)
{

	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &fno);
	if (fresult != FR_OK)
	{
		char *buf = pvPortMalloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* does not exists\r\n\n", name);
		Print_Msg (buf);
	    vPortFree(buf);
	    return fresult;
	}

	else
	{
	    /* Create a file with read write access and open it */
	    fresult = f_open(&fil, name, FA_OPEN_EXISTING | FA_WRITE);
	    if (fresult != FR_OK)
	    {
	    	char *buf = pvPortMalloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in opening file *%s*\r\n\n", fresult, name);
	    	Print_Msg(buf);
	        vPortFree(buf);
	        return fresult;
	    }

	    else
	    {
	    	fresult = f_write(&fil, data, strlen(data), &bw);
	    	if (fresult != FR_OK)
	    	{
	    		char *buf = pvPortMalloc(100*sizeof(char));
	    		sprintf (buf, "ERROR!!! No. %d while writing to the FILE *%s*\r\n\n", fresult, name);
	    		Print_Msg(buf);
	    		vPortFree(buf);
	    	}

	    	/* Close file */
	    	fresult = f_close(&fil);
	    	if (fresult != FR_OK)
	    	{
	    		char *buf = pvPortMalloc(100*sizeof(char));
	    		sprintf (buf, "ERROR!!! No. %d in closing file *%s* after writing it\r\n\n", fresult, name);
	    		Print_Msg(buf);
	    		vPortFree(buf);
	    	}
	    	else
	    	{
	    		char *buf = pvPortMalloc(100*sizeof(char));
	    		sprintf (buf, "File *%s* is WRITTEN and CLOSED successfully\r\n", name);
	    		Print_Msg(buf);
	    		vPortFree(buf);
	    	}
	    }
	    return fresult;
	}
}

FRESULT Read_File (char *name)
{
	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &fno);
	if (fresult != FR_OK)
	{
		char *buf = pvPortMalloc(100*sizeof(char));
		sprintf (buf, "ERRROR!!! *%s* does not exists\r\n\n", name);
		Print_Msg (buf);
		vPortFree(buf);
	    return fresult;
	}

	else
	{
		/* Open file to read */
		fresult = f_open(&fil, name, FA_READ);

		if (fresult != FR_OK)
		{
			char *buf = pvPortMalloc(100*sizeof(char));
			sprintf (buf, "ERROR!!! No. %d in opening file *%s*\r\n\n", fresult, name);
		    Print_Msg(buf);
		    vPortFree(buf);
		    return fresult;
		}

		/* Read data from the file
		* see the function details for the arguments */

		char *buffer = pvPortMalloc(sizeof(f_size(&fil)));
		fresult = f_read (&fil, buffer, f_size(&fil), &br);
		if (fresult != FR_OK)
		{
			char *buf = pvPortMalloc(100*sizeof(char));
			vPortFree(buffer);
		 	sprintf (buf, "ERROR!!! No. %d in reading file *%s*\r\n\n", fresult, name);
		  	Print_Msg(buffer);
		  	vPortFree(buf);
		}

		else
		{
			Print_Msg(buffer);
			vPortFree(buffer);

			/* Close file */
			fresult = f_close(&fil);
			if (fresult != FR_OK)
			{
				char *buf = pvPortMalloc(100*sizeof(char));
				sprintf (buf, "ERROR!!! No. %d in closing file *%s*\r\n\n", fresult, name);
				Print_Msg(buf);
				vPortFree(buf);
			}
			else
			{
				char *buf = pvPortMalloc(100*sizeof(char));
				sprintf (buf, "File *%s* CLOSED successfully\r\n", name);
				Print_Msg(buf);
				vPortFree(buf);
			}
		}
	    return fresult;
	}
}

FRESULT Create_File (char *name)
{
	fresult = f_stat (name, &fno);
	if (fresult == FR_OK)
	{
		char *buf = pvPortMalloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* already exists!!!!\n use Update_File \r\n\n",name);
		Print_Msg(buf);
		vPortFree(buf);
	    return fresult;
	}
	else
	{
		fresult = f_open(&fil, name, FA_CREATE_ALWAYS|FA_READ|FA_WRITE);
		if (fresult != FR_OK)
		{
			char *buf = pvPortMalloc(100*sizeof(char));
			sprintf (buf, "ERROR!!! No. %d in creating file *%s*\r\n\n", fresult, name);
			Print_Msg(buf);
			vPortFree(buf);
		    return fresult;
		}
		else
		{
			char *buf = pvPortMalloc(100*sizeof(char));
			sprintf (buf, "*%s* created successfully\n Now use Write_File to write data\r\n",name);
			Print_Msg(buf);
			vPortFree(buf);
		}

		fresult = f_close(&fil);
		if (fresult != FR_OK)
		{
			char *buf = pvPortMalloc(100*sizeof(char));
			sprintf (buf, "ERROR No. %d in closing file *%s*\r\n\n", fresult, name);
			Print_Msg(buf);
			vPortFree(buf);
		}
		else
		{
			char *buf = pvPortMalloc(100*sizeof(char));
			sprintf (buf, "File *%s* CLOSED successfully\r\n", name);
			Print_Msg(buf);
			vPortFree(buf);
		}
	}
    return fresult;
}

FRESULT Update_File (char *name, char *data)
{
	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &fno);
	if (fresult != FR_OK)
	{
		char *buf = pvPortMalloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* does not exists\r\n\n", name);
		Print_Msg (buf);
		vPortFree(buf);
	    return fresult;
	}

	else
	{
		 /* Create a file with read write access and open it */
	    fresult = f_open(&fil, name, FA_OPEN_APPEND | FA_WRITE);
	    if (fresult != FR_OK)
	    {
	    	char *buf = pvPortMalloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in opening file *%s*\r\n\n", fresult, name);
	    	Print_Msg(buf);
	        vPortFree(buf);
	        return fresult;
	    }

	    /* Writing text */
	    fresult = f_write(&fil, data, strlen (data), &bw);
	    if (fresult != FR_OK)
	    {
	    	char *buf = pvPortMalloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in writing file *%s*\r\n\n", fresult, name);
	    	Print_Msg(buf);
	    	vPortFree(buf);
	    }

	    else
	    {
	    	char *buf = pvPortMalloc(100*sizeof(char));
	    	sprintf (buf, "*%s* UPDATED successfully\n", name);
	    	Print_Msg(buf);
	    	vPortFree(buf);
	    }

	    /* Close file */
	    fresult = f_close(&fil);
	    if (fresult != FR_OK)
	    {
	    	char *buf = pvPortMalloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in closing file *%s*\r\n\n", fresult, name);
	    	Print_Msg(buf);
	    	vPortFree(buf);
	    }
	    else
	    {
	    	char *buf = pvPortMalloc(100*sizeof(char));
	    	sprintf (buf, "File *%s* CLOSED successfully\r\n", name);
	    	Print_Msg(buf);
	    	vPortFree(buf);
	     }
	}
    return fresult;
}

FRESULT Remove_File (char *name)
{
	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &fno);
	if (fresult != FR_OK)
	{
		char *buf = pvPortMalloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* does not exists\r\n\n", name);
		Print_Msg (buf);
		vPortFree(buf);
		return fresult;
	}

	else
	{
		fresult = f_unlink (name);
		if (fresult == FR_OK)
		{
			char *buf = pvPortMalloc(100*sizeof(char));
			sprintf (buf, "*%s* has been removed successfully\r\n", name);
			Print_Msg (buf);
			vPortFree(buf);
		}

		else
		{
			char *buf = pvPortMalloc(100*sizeof(char));
			sprintf (buf, "ERROR No. %d in removing *%s*\r\n\n",fresult, name);
			Print_Msg (buf);
			vPortFree(buf);
		}
	}
	return fresult;
}

FRESULT Create_Dir (char *name)
{
    fresult = f_mkdir(name);
    if (fresult == FR_OK)
    {
    	char *buf = pvPortMalloc(100*sizeof(char));
    	sprintf (buf, "*%s* has been created successfully\r\n", name);
    	Print_Msg (buf);
    	vPortFree(buf);
    }
    else
    {
    	char *buf = pvPortMalloc(100*sizeof(char));
    	sprintf (buf, "ERROR No. %d in creating directory *%s*\r\n\n", fresult,name);
    	Print_Msg(buf);
    	vPortFree(buf);
    }
    return fresult;
}

void Check_SD_Space (void)
{
    /* Check free space */
    f_getfree("", &fre_clust, &pfs);

    total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
    char *buf = pvPortMalloc(30*sizeof(char));
    sprintf (buf, "SD CARD Total Size: \t%lu\r\n",total);
    Print_Msg(buf);
    vPortFree(buf);
    free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
    buf = pvPortMalloc(30*sizeof(char));
    sprintf (buf, "SD CARD Free Space: \t%lu\r\n",free_space);
    Print_Msg(buf);
    vPortFree(buf);
}

