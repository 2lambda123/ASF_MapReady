/*
  get_ceos_names:

  Functions that match CEOS files with each other primarily using their
  extensions.
*/

#include "asf.h"
#include "get_ceos_names.h"
#include "meta_init.h"

const char ceos_metadata_extensions[][12] =
  {"","LEA_ TRA_",".sarl .sart",".L",".LDR",".ldr","LEA.","lea.","LED-"};
const char ceos_data_extensions[][12] =
  {"","DAT_",".sard",".D",".RAW",".raw","DAT.","dat.","IMG-"};


/******************************************************************************
 * get_ceos_metadata_name:
 * Given the name of a file (potentially with path in front of it), determine
 * if it is a CEOS metadata file (depending on our accepted CEOS extensions).
 * If so populate metaName with the appropriate name and return the appropriate
 * ENUM ceos_metadata_ext_t value.  */
ceos_metadata_ext_t get_ceos_metadata_name(const char *ceosName, char **metaName,
					   int *trailer)
{
  char dirName[256], fileName[256];
  char leaderTemp[1024], trailerTemp[1024];
  char baseName[256];
  char ext[256];
  char *leaderExt, *trailerExt;
  FILE *leaderFP, *trailerFP;
  int begin=NO_CEOS_METADATA+1, end=NUM_CEOS_METADATA_EXTS;
  int ii, index;

  leaderExt = (char *) MALLOC(sizeof(char)*25);
  trailerExt = (char *) MALLOC(sizeof(char)*25);

  /* Separate the filename from the path (if there's a path there) */
  split_dir_and_file(ceosName, dirName, fileName);

  for (ii=begin; ii<end; ii++) {
    *trailer = 0;
    trailerExt = strchr(ceos_metadata_extensions[ii], ' ');
    if (trailerExt) {
      index = trailerExt - ceos_metadata_extensions[ii];
      strncpy(leaderExt, ceos_metadata_extensions[ii], index);
      leaderExt[index] = 0;
      trailerExt++;
      *trailer = 1;
    }
    else
      strcpy(leaderExt, ceos_metadata_extensions[ii]);

    int strEnd = strlen(leaderExt) - 1;
    
    /* First check for suffix style extensions */
    if (leaderExt[0] == EXTENSION_SEPARATOR) {
      /* Assume ceosName came to the function as a base name */
      if (trailerExt) {
	sprintf(leaderTemp, "%s%s%s", dirName, fileName, leaderExt);
	sprintf(trailerTemp, "%s%s%s", dirName, fileName, trailerExt);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL &&
	    (trailerFP = fopen(trailerTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  fclose(trailerFP);
	  strcpy(metaName[1], trailerTemp);
	  return ii;
	}
      }
      else {
        sprintf(leaderTemp, "%s%s%s", dirName, fileName, leaderExt);
        if ((leaderFP = fopen(leaderTemp, "r"))!=NULL) {
          fclose(leaderFP);
          strcpy(metaName[0], leaderTemp);
          return ii;
        }
      }
      /* Hmmm, didn't work, maybe it's got an extension on there already,
       * nix it and try again */
      split_base_and_ext(fileName, APPENDED_EXTENSION, '.', baseName, ext);
      if (trailerExt) {
	sprintf(leaderTemp, "%s%s%s", dirName, baseName, leaderExt);
	sprintf(trailerTemp, "%s%s%s", dirName, baseName, trailerExt);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL &&
	    (trailerFP = fopen(trailerTemp, "r"))!= NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  fclose(trailerFP);
	  strcpy(metaName[1], trailerTemp);
	  return ii;
	}
      }
      else {
        sprintf(leaderTemp, "%s%s%s", dirName, baseName, leaderExt);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  return ii;
	}
      }
    }
    /* Second look for prefix style extensions '.' */
    else if (leaderExt[strEnd] == EXTENSION_SEPARATOR) {
      /* Assume ceosName came to the function as a base name */
      if (trailerExt) {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, fileName);
	sprintf(trailerTemp, "%s%s%s", dirName, trailerExt, fileName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL &&
	    (trailerFP = fopen(trailerTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  fclose(trailerFP);
	  strcpy(metaName[1], trailerTemp);
	  return ii;
	}
      }
      else {
        sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, fileName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL) {
          fclose(leaderFP);
          strcpy(metaName[0], leaderTemp);
          return ii;
        }
      }
      /* Hmmm, didn't work, maybe it's got an extension on there already,
       * nix it and try again */
      split_base_and_ext(fileName, PREPENDED_EXTENSION, '.', baseName, ext);
      if (trailerExt) {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, baseName);
	sprintf(trailerTemp, "%s%s%s", dirName, trailerExt, baseName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL &&
	    (trailerFP = fopen(trailerTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  fclose(trailerFP);
	  strcpy(metaName[1], trailerTemp);
	  return ii;
	}
      }
      else {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, baseName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  return ii;
	}
      }
    }
    /* Third look for prefix style extensions '-' */
    else if (leaderExt[strEnd] == ALOS_EXTENSION_SEPARATOR) {
      /* Assume ceosName came to the function as a base name */
      if (trailerExt) {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, fileName);
	sprintf(trailerTemp, "%s%s%s", dirName, trailerExt, fileName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL &&
	    (trailerFP = fopen(trailerTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  fclose(trailerFP);
	  strcpy(metaName[1], trailerTemp);
	  return ii;
	}
      }
      else {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, fileName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  return ii;
	}
      }
      /* Hmmm, didn't work, maybe it's got an extension on there already,
       * nix it and try again */
      split_base_and_ext(fileName, PREPENDED_EXTENSION, '-', baseName, ext);
      if (trailerExt) {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, baseName);
	sprintf(trailerTemp, "%s%s%s", dirName, trailerExt, baseName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL &&
	    (trailerFP = fopen(trailerTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  fclose(trailerFP);
	  strcpy(metaName[0], trailerTemp);
	  return ii;
	}
      }
      else {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, baseName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  return ii;
	}
      }
    }    
    /* Fourth look for prefix style extensions '_' */
    else if (leaderExt[strEnd] == CAN_EXTENSION_SEPARATOR) {
      /* Assume ceosName came to the function as a base name */
      if (trailerExt) {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, fileName);
	sprintf(trailerTemp, "%s%s%s", dirName, trailerExt, fileName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL &&
	    (trailerFP = fopen(trailerTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  fclose(trailerFP);
	  strcpy(metaName[1], trailerTemp);
	  return ii;
	}
      }
      else {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, fileName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  return ii;
	}
      }
      /* Hmmm, didn't work, maybe it's got an extension on there already,
       * nix it and try again */
      split_base_and_ext(fileName, PREPENDED_EXTENSION, '_', baseName, ext);
      if (trailerExt) {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, baseName);
	sprintf(trailerTemp, "%s%s%s", dirName, trailerExt, baseName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL &&
	    (trailerFP = fopen(trailerTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  fclose(trailerFP);
	  strcpy(metaName[1], trailerTemp);
	  return ii;
	}
      }
      else {
	sprintf(leaderTemp, "%s%s%s", dirName, leaderExt, baseName);
	if ((leaderFP = fopen(leaderTemp, "r"))!=NULL) {
	  fclose(leaderFP);
	  strcpy(metaName[0], leaderTemp);
	  return ii;
	}
      }
    }
  }
  /* If we haven't returned yet there ain't no metadata file */
  return NO_CEOS_METADATA;
}

/******************************************************************************
 * require_ceos_metadata:
 * If get_ceos_metadata_name fails to find a file, then exit the program.
 * Otherwise act like get_ceos_metadata_name  */
ceos_metadata_ext_t require_ceos_metadata(const char *ceosName, char **metaName,
					  int *trailer)
{
  ceos_metadata_ext_t ret = 
    get_ceos_metadata_name(ceosName, metaName, trailer);

  /* If we didn't find anything, report & leave */
  if (ret == NO_CEOS_METADATA) {
    char extensionList[128];
    int andFlag=TRUE;
    int begin=NO_CEOS_METADATA+1, end=NUM_CEOS_METADATA_EXTS;
    int ii;

    /* Prepare a very readable list of possible extensions */
    sprintf(extensionList,"%s",ceos_metadata_extensions[begin++]);
    if (end-begin == 0)
      andFlag=FALSE;
    else
      end--;
    for (ii=begin; ii<end; ii++) {
      sprintf(extensionList,"%s, %s",extensionList,
                                     ceos_metadata_extensions[ii]);
    }
    if (andFlag)
      sprintf(extensionList,"%s, and %s",extensionList,
                                         ceos_metadata_extensions[ii]);

    /* Report to user & exit */
    sprintf(logbuf,
            "**************************** ERROR! ****************************\n"
            "*   This program was looking for the CEOS style metadata file,\n"
            "*   %s\n"
            "*   That file either does not exist or cannot be read.\n"
            "*   Expected metadata file extensions are:\n"
            "*   %s\n"
            "****************************************************************\n",
            ceosName, extensionList);
    if (logflag)   {printLog(logbuf);}
    printf(logbuf);
    exit(EXIT_FAILURE);
  }

  /* If we found a file, return its extension type! */
  else
    return ret;
}


/******************************************************************************
 * get_ceos_data_name:
 * Given the name of a file (potentially with path in front of it), determine
 * if it is a CEOS data file (depending on our accepted CEOS extensions). If
 * so populate dataName with the appropriate name and return the appropriate
 * ENUM ceos_data_ext_t value.  */
ceos_data_ext_t get_ceos_data_name(const char *ceosName, char **dataName, int *nBands)
{
  char dirName[256], fileName[256];
  char dataTemp[1024];
  char baseName[256];
  char ext[256];
  FILE *dataFP;
  int begin=NO_CEOS_DATA+1, end=NUM_CEOS_DATA_EXTS;
  int ii, kk;

  *nBands = 0;

  /* Separate the filename from the path (if there's a path there) */
  split_dir_and_file(ceosName, dirName, fileName);

  for (ii=begin; ii<end; ii++) {
    int strEnd = strlen(ceos_data_extensions[ii]) - 1;

    /* First check for suffix style extensions */
    if (ceos_data_extensions[ii][0] == EXTENSION_SEPARATOR) {
      /* Assume ceosName came to the function as a base name */
      sprintf(dataTemp,"%s%s%s",dirName,fileName,ceos_data_extensions[ii]);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[0],dataTemp);
	*nBands = 1;
        return ii;
      }
      /* Hmmm, didn't work, maybe it's got an extension on there already,
       * nix it and try again */
      split_base_and_ext(fileName, APPENDED_EXTENSION, '.', baseName, ext);
      sprintf(dataTemp,"%s%s%s",dirName,baseName,ceos_data_extensions[ii]);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[0],dataTemp);
	*nBands = 1;
        return ii;
      }
    }
    /* Second look for prefix style extensions '.' */
    else if (ceos_data_extensions[ii][strEnd] == EXTENSION_SEPARATOR) {
      /* Assume ceosName came to the function as a base name */
      sprintf(dataTemp,"%s%s%s",dirName,ceos_data_extensions[ii],baseName);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[0],dataTemp);
	*nBands = 1;
        return ii;
      }
      /* Hmmm, didn't work, maybe it's got an extension on there already,
       * nix it and try again */
      split_base_and_ext(fileName, PREPENDED_EXTENSION, '.', baseName, ext);
      sprintf(dataTemp,"%s%s%s",dirName,ceos_data_extensions[ii],baseName);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[0],dataTemp);
	*nBands = 1;
        return ii;
      }
    }
    /* Third look for prefix style extensions '.' */
    else if (ceos_data_extensions[ii][strEnd] == ALOS_EXTENSION_SEPARATOR) {
      /* Assume ceosName came to the function as a file name */
      sprintf(dataTemp,"%s%sHH-%s",dirName,ceos_data_extensions[ii],fileName);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[*nBands],dataTemp);
        (*nBands)++;
      }
      sprintf(dataTemp,"%s%sHV-%s",dirName,ceos_data_extensions[ii],fileName);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[*nBands],dataTemp);
        (*nBands)++;
      }
      sprintf(dataTemp,"%s%sVH-%s",dirName,ceos_data_extensions[ii],fileName);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[*nBands],dataTemp);
        (*nBands)++;
      }
      sprintf(dataTemp,"%s%sVV-%s",dirName,ceos_data_extensions[ii],fileName);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[*nBands],dataTemp);
        (*nBands)++;
      }
      for (kk=1; kk<10; kk++) {
	sprintf(dataTemp,"%s%s0%d-%s",dirName,ceos_data_extensions[ii],kk,fileName);
	if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
	  fclose(dataFP);
	  strcpy(dataName[*nBands],dataTemp);
	  (*nBands)++;
	}
      }
      if (*nBands == 0) {
	sprintf(dataTemp,"%s%s%s",dirName,ceos_data_extensions[ii],fileName);
	if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
	  fclose(dataFP);
	  strcpy(dataName[*nBands],dataTemp);
	  (*nBands) = 1;
	}
      }
      if (*nBands)
	return ii;
    }
    /* Fourth look for prefix style extensions '_' */
    else if (ceos_data_extensions[ii][strEnd] == CAN_EXTENSION_SEPARATOR) {
      /* Assume ceosName came to the function as a base name */
      sprintf(dataTemp,"%s%s%s",dirName,ceos_data_extensions[ii],baseName);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[0],dataTemp);
        *nBands = 1;
        return ii;
      }
      /* Hmmm, didn't work, maybe it's got an extension on there already,
       * nix it and try again */
      split_base_and_ext(fileName, PREPENDED_EXTENSION, '_', baseName, ext);
      sprintf(dataTemp,"%s%s%s",dirName,ceos_data_extensions[ii],baseName);
      if ((dataFP=fopen(dataTemp,"r"))!=NULL) {
        fclose(dataFP);
        strcpy(dataName[0],dataTemp);
        *nBands = 1;
        return ii;
      }
    }
 }
  /* If we haven't returned yet there ain't no data file */
  return NO_CEOS_DATA;
}

/******************************************************************************
 * require_ceos_data:
 * If get_ceos_data_name fails to find a file, then exit the program.
 * Otherwise act like get_ceos_data_name  */
ceos_data_ext_t require_ceos_data(const char *ceosName, char **dataName, int *nBands)
{
  ceos_data_ext_t ret = get_ceos_data_name(ceosName, dataName, nBands);

  /* If we didn't find anything, report & leave */
  if (ret == NO_CEOS_DATA) {
    char extensionList[128];
    int andFlag=TRUE;
    int begin=NO_CEOS_DATA+1, end=NUM_CEOS_DATA_EXTS;
    int ii;

    /* Prepare a very readable list of possible extensions */
    sprintf(extensionList,"%s",ceos_data_extensions[begin++]);
    if (end-begin == 0)
      andFlag=FALSE;
    else
      end--;
    for (ii=begin; ii<end; ii++) {
      sprintf(extensionList,"%s, %s",extensionList,
                                     ceos_data_extensions[ii]);
    }
    if (andFlag)
      sprintf(extensionList,"%s, and %s",extensionList,
                                         ceos_data_extensions[ii]);

    /* Report to user & exit */
    sprintf(logbuf,
           "**************************** ERROR! ****************************\n"
           "*   This program was looking for the CEOS style data file,\n"
           "*   %s\n"
           "*   That file either does not exist or cannot be read.\n"
           "*   Expected data file extensions are:\n"
           "*   %s\n"
           "****************************************************************\n",
           ceosName, extensionList);
    if (logflag)   {printLog(logbuf);}
    printf(logbuf);
    exit(EXIT_FAILURE);
  }

  /* If we found a file, return its extension type! */
  else
    return ret;
}


/******************************************************************************
 * get_ceos_names:
 * Given the name of a file (potentially with path in front of it), determine
 * if it is one of a CEOS file pair (depending on our accepted CEOS file
 * extensions). If so populate dataName & metaName with the appropriate names
 * and return the appropriate ENUM ceos_file_pairs_t value.*/
ceos_file_pairs_t get_ceos_names(const char *ceosName, char **dataName,
                                 char **metaName, int *nBands, int *trailer)
{
  ceos_metadata_ext_t metadata_ext;
  ceos_data_ext_t     data_ext;

  strcpy(*metaName, "");
  strcpy(*dataName, "");
  metadata_ext = get_ceos_metadata_name(ceosName, metaName, trailer);
  data_ext = get_ceos_data_name(ceosName, dataName, nBands);

  if (data_ext == CEOS_DAT && metadata_ext == CEOS_LEA_TRA)
    return CEOS_DAT_LEA_TRA_TRIPLE;

  if (data_ext == CEOS_sard && metadata_ext == CEOS_sarl_sart)
    return CEOS_sard_sarl_sart_TRIPLE;
  
  if (data_ext == CEOS_D && metadata_ext == CEOS_L)
    return CEOS_D_L_PAIR;

  if (data_ext == CEOS_RAW && metadata_ext == CEOS_LDR)
    return CEOS_RAW_LDR_PAIR;

  if (data_ext == CEOS_dat && metadata_ext == CEOS_lea)
    return CEOS_dat_lea_PAIR;

  if (data_ext == CEOS_raw && metadata_ext == CEOS_ldr)
    return CEOS_RAW_LDR_PAIR;

  if (data_ext == CEOS_DAT && metadata_ext == CEOS_LEA)
    return CEOS_dat_lea_PAIR;
  
  if (data_ext == CEOS_dat && metadata_ext == CEOS_lea)
    return CEOS_dat_lea_PAIR;
  
  if (data_ext == CEOS_IMG && metadata_ext == CEOS_LED)
    return CEOS_IMG_LED_PAIR;

  return NO_CEOS_FILE_PAIR;
}

/******************************************************************************
 * require_ceos_pair:
 * Do as get_ceos_names would unless there is no pair in which case exit the
 * program with a failure.  */
ceos_file_pairs_t require_ceos_pair(const char *ceosName, char **dataName, 
				    char **metaName, int *nBands, int *trailer)
{
  char extensionList[128];
  int andFlag=TRUE;
  int begin=NO_CEOS_FILE_PAIR+1, end=NUM_CEOS_FILE_PAIRS;
  int ii;

  // Check for metadata separated in leader and trailer file
  if (require_ceos_data(ceosName, dataName, nBands) == CEOS_DAT_ &&
      require_ceos_metadata(ceosName, metaName, trailer) == CEOS_LEA_TRA)
    return CEOS_DAT_LEA_TRA_TRIPLE;

  if (require_ceos_data(ceosName, dataName, nBands) == CEOS_sard &&
      require_ceos_metadata(ceosName, metaName, trailer) == CEOS_sarl_sart)
    return CEOS_sard_sarl_sart_TRIPLE;

  // Check for regular metadata files
  if (   require_ceos_data(ceosName, dataName, nBands) == CEOS_D
      && require_ceos_metadata(ceosName, metaName, trailer) == CEOS_L)
    return CEOS_D_L_PAIR;

  if (   require_ceos_data(ceosName, dataName, nBands) == CEOS_RAW
      && require_ceos_metadata(ceosName, metaName, trailer) == CEOS_LDR)
    return CEOS_RAW_LDR_PAIR;

  if (   require_ceos_data(ceosName, dataName, nBands) == CEOS_raw
      && require_ceos_metadata(ceosName, metaName, trailer) == CEOS_ldr)
    return CEOS_raw_ldr_PAIR;

  if (   require_ceos_data(ceosName, dataName, nBands) == CEOS_DAT
      && require_ceos_metadata(ceosName, metaName, trailer) == CEOS_LEA)
    return CEOS_DAT_LEA_PAIR;

  if (   require_ceos_data(ceosName, dataName, nBands) == CEOS_dat
      && require_ceos_metadata(ceosName, metaName, trailer) == CEOS_lea)
    return CEOS_dat_lea_PAIR;

  if (   require_ceos_data(ceosName, dataName, nBands) == CEOS_IMG
      && require_ceos_metadata(ceosName, metaName, trailer) == CEOS_LED)
    return CEOS_IMG_LED_PAIR;

/****** We should never actually get here. The above code should either ******
 ****** return or exit with an error message, BUT... just in case       ******/

  /* Prepare a very readable list of possible extension pairs */
  sprintf(extensionList,"(%s %s)",ceos_data_extensions[begin],
                                  ceos_metadata_extensions[begin]);
  begin++;
  if (end-begin == 0)
    andFlag=FALSE;
  else
    end--;
  for (ii=begin; ii<end; ii++) {
    sprintf(extensionList,"%s, (%s %s)",extensionList,
                                        ceos_data_extensions[ii],
                                        ceos_metadata_extensions[ii]);
  }
  if (andFlag)
    sprintf(extensionList,"%s, and (%s %s)",extensionList,
                                            ceos_data_extensions[ii],
                                            ceos_metadata_extensions[ii]);

  /* Report to user & exit */
  sprintf(logbuf,
          "**************************** ERROR! ****************************\n"
          "*   This program was looking for the CEOS style SAR files,\n"
          "*   %s and its associated file.\n"
          "*   One or both files either do not exist or cannot be read.\n"
          "*   Expected fileset extensions are:\n"
          "*   %s\n"
          "****************************************************************\n",
          ceosName, extensionList);
  if (logflag)   {printLog(logbuf);}
  printf(logbuf);
  exit(EXIT_FAILURE);
}
