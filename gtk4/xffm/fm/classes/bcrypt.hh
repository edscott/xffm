#ifndef BCRYPT_HH
#define BCRYPT_HH

/* ====================================================================
 * Copyright (c) 2002 Johnny Shelley.  All rights reserved.
 *
 * Bcrypt is licensed under the BSD software license. See the file 
 * called 'LICENSE' that you should have received with this software
 * for details
 *
 *  C++ class by Edscott Wilson Garcia 2025, same license as above.
 * ====================================================================
 */

// include blowfish.hh before this file.??

namespace xf{

  class Bcrypt : public BlowFish {
    public:

    private:
      BCoptions initoptions(BCoptions options) {
        options.remove = REMOVE;
        options.standardout = STANDARDOUT;
        options.compression = COMPRESS;	/* leave this on by default	*/
        options.type = 127;
        options.origsize = 0;
        options.securedelete = SECUREDELETE;
        return(options);
      }

      uLong BFEncrypt(char **input, unsigned char *key, uLong sz, BCoptions *options) {
        uInt32 L, R;
        uLong i;
        BLOWFISH_CTX ctx;
        int j;
        unsigned char *myEndian = NULL;
        j = sizeof(uInt32);

        getEndian(&myEndian);

        memmove(*input+2, *input, sz);

        memcpy(*input, myEndian, 1);
        memcpy(*input+1, &options->compression, 1);

        sz += 2;    /* add room for endian and compress flags */

        Blowfish_Init (&ctx, key, MAXKEYBYTES);

        for (i = 2; i < sz; i+=(j*2)) {	/* start just after tags */
          memcpy(&L, *input+i, j);
          memcpy(&R, *input+i+j, j);
          Blowfish_Encrypt(&ctx, &L, &R);
          memcpy(*input+i, &L, j);
          memcpy(*input+i+j, &R, j);
        }

        if (options->compression == 1) {
          if ((*input = (char *)realloc(*input, sz + j + 1)) == NULL)
            memerror();

          memset(*input+sz, 0, j + 1);
          memcpy(*input+sz, &options->origsize, j);
          sz += j;  /* make room for the original size      */
        }

        free(myEndian);
        return(sz);
      }

      uLong BFDecrypt(char **input, char *key, char *key2, uLong sz, 
        BCoptions *options) {
        uInt32 L, R;
        uLong i;
        BLOWFISH_CTX ctx;
        int j, swap = 0;
        unsigned char *myEndian = NULL;
        unsigned char *mykey = NULL;

        j = sizeof(uInt32);

        if ((mykey = (unsigned char *)malloc(MAXKEYBYTES + 1)) == NULL)
          memerror();

        memset(mykey, 0, MAXKEYBYTES + 1);

        if ((swap = testEndian(*input)) == 1)
          memcpy(mykey, key2, MAXKEYBYTES);
        else
          memcpy(mykey, key, MAXKEYBYTES);

        memcpy(&options->compression, *input+1, 1);

        if (options->compression == 1) {
          memcpy(&options->origsize, *input+(sz - j), j);
          sz -= j;  /* dump the size tag    */
        }

        sz -= 2;    /* now dump endian and compress flags   */

        Blowfish_Init (&ctx, mykey, MAXKEYBYTES);

        for (i = 0; i < sz; i+=(j*2)) {
          memcpy(&L, *input+i+2, j);
          memcpy(&R, *input+i+j+2, j);

          if (swap == 1) {
            L = swapEndian(L);
            R = swapEndian(R);
          }

          Blowfish_Decrypt(&ctx, &L, &R);

          if (swap == 1) {
            L = swapEndian(L);
            R = swapEndian(R);
          }

          memcpy(*input+i, &L, j);
          memcpy(*input+i+j, &R, j);
        }

        while (memcmp(*input+(sz-1), "\0", 1) == 0)  /* strip excess nulls   */
          sz--;                                       /* from decrypted files */

        sz -= MAXKEYBYTES;

        if (memcmp(*input+sz, mykey, MAXKEYBYTES) != 0)
          return(0);

        free(mykey);
        free(myEndian);
        return(sz);
      }

      /////////////// Bfkeys      
      char * getkey(int type){
        char *key, *key2, overflow[2], *ch;

        if ((key = (char *)malloc(MAXKEYBYTES + 2)) == NULL)
          memerror();

        memset(key, 0, MAXKEYBYTES + 2);

        fprintf(stderr, "Encryption key:");
        fgets(key, MAXKEYBYTES + 1, stdin);

        /* blowfish requires 32 bits, I want 64. deal w/ it	*/
        while (strlen(key) < 9 && type == ENCRYPT) {	/* \n is still tacked on */
          fprintf(stderr, "Key must be at least 8 characters\n");
          memset(key, 0, MAXKEYBYTES + 2);
          fprintf(stderr, "Encryption key:");
          fgets(key, MAXKEYBYTES + 1, stdin);
        }

        if (memchr(key, (char) 10, MAXKEYBYTES + 1) == NULL) {
          while (fread(overflow, 1, 1, stdin) > 0) {
            if (memcmp(overflow, "\n", 1) == 0)
              break;
          }
        }

        if (type == ENCRYPT) {
          if ((key2 = (char *)malloc(MAXKEYBYTES + 2)) == NULL)
            memerror();

          memset(key2, 0, MAXKEYBYTES + 2);
          fprintf(stderr, "\nAgain:");
          fgets(key2, MAXKEYBYTES + 1, stdin);
        
          if (strcmp(key, key2)) {
            fprintf(stderr, "\nKeys don't match!\n");
            exit(1);
          }
          memset(key2, 0, strlen(key2));
          free(key2);
        }

        if ((ch = (char *)memchr(key, (char) 10, strlen(key))) != NULL)
          memset(ch, 0, 1);

        fprintf(stderr, "\n");

        return(key);
      }

      void mutateKey(unsigned char **key, char **key2) {
        uInt32 L, R, l, r;
        BLOWFISH_CTX ctx;
        char *newkey, *newkey2;
        int i, j;

        j = sizeof(uInt32);

        Blowfish_Init(&ctx, *key, strlen((const char *)*key));

        memcpy(&L, *key, j);
        memcpy(&R, *key+j, j);
        memset(*key, 0, MAXKEYBYTES + 1);

        l = L;
        r = R;

        if ((*key2 = (char *)malloc(MAXKEYBYTES + 1)) == NULL)
          memerror();
        if ((newkey = (char *)malloc(MAXKEYBYTES + 1)) == NULL)
          memerror();
        if ((newkey2 = (char *)malloc(MAXKEYBYTES + 1)) == NULL)
          memerror();

        memset(*key2, 0, MAXKEYBYTES + 1);
        memset(newkey, 0, MAXKEYBYTES + 1);
        memset(newkey2, 0, MAXKEYBYTES + 1);

        for (i=0; i < MAXKEYBYTES; i+=(j*2)) {
          Blowfish_Encrypt(&ctx, &L, &R);
          memcpy(newkey+i, &L, j);
          memcpy(newkey+i+j, &R, j);
        }

        for (i=0; i < MAXKEYBYTES; i+=(j*2)) {
          l = swapEndian(l);
          r = swapEndian(r);

          Blowfish_Encrypt(&ctx, &l, &r);

          l = swapEndian(l);
          r = swapEndian(r);

          memcpy(newkey2+i, &l, j);
          memcpy(newkey2+i+j, &r, j);
        }

        memcpy(*key, newkey, MAXKEYBYTES);
        memcpy(*key2, newkey2, MAXKEYBYTES);
        free(newkey);
      }

      /////////////// Bfrwfile      
      int getremain(uLong sz, int dv) {
        int r;
         
        r = sz / dv;
        r++;
        r = r * dv;
        r = r - sz;
        return(r);
      }

      uLong padInput(char **input, uLong sz) {
        int r, j;

        j = sizeof(uInt32) * 2;

        if (sz >= j)
          r = getremain(sz, j);
        else
          r = j - sz;

        if ( r < j) {
          if ((*input = (char *)realloc(*input, sz + r + 1)) == NULL)
            memerror();

          memset(*input+sz, 0, r + 1);
          sz+=r;
        }

        return(sz);
      }

      uLong attachKey(char **input, char *key, uLong sz) {

        /* +3 so we have room for info tags at the beginning of the file */
        if ((*input = (char *)realloc(*input, sz + MAXKEYBYTES + 3)) == NULL)
          memerror();

        memcpy(*input+sz, key, MAXKEYBYTES);
        sz += MAXKEYBYTES;

        return(sz);
      }

      uLong readfile(char *infile, char **input, int type, char *key, 
        struct stat statbuf) {
        FILE *fd;
        int readsize;
        uLong sz = 0;

        readsize = statbuf.st_size + 1;

        fd = fopen(infile, "rb");
        if (!fd) {
          fprintf(stderr, "Unable to open file %s\n", infile);
          return(-1);
        }

        if ((*input = (char *)malloc(readsize + sz + 1)) == NULL) 
          memerror();

        memset(*input+sz, 0, readsize);
        sz += fread(*input+sz, 1, readsize - 1, fd);

        fclose(fd);

        return(sz);
      }

      uLong writefile(char *outfile, char *output, uLong sz, 
        BCoptions options, struct stat statbuf) {
        FILE *fd;

        if (options.standardout == 1) 
          fd = stdout;
        else
          fd = fopen(outfile, "wb");

        if (!fd) {
          fprintf(stderr, "Unable to create file %s\n", outfile);
          exit(1);
        }

        if (fwrite(output, 1, sz, fd) != sz) {
          fprintf(stderr, "Out of space while writing file %s\n", outfile);
          exit(1);
        }

        if (!options.standardout) {
          fclose(fd);
          chmod(outfile, statbuf.st_mode);
        }

        return(0);
      }

      int deletefile(char *file, BCoptions options, char *key, struct stat statbuf) {
        int lsize;
        long g;
        uLong j = 0, k = 0;
        signed char i;
        char *state, *garbage;
        FILE *fd;

        if (options.securedelete > 0) {
          lsize = sizeof(long);
          k = (statbuf.st_size / lsize) + 1;
          if ((state = (char *)malloc(257)) == NULL)
            memerror();

          initstate((unsigned long) key, state, 256);
          if ((garbage = (char *)malloc(lsize + 1)) == NULL)
            memerror();

          fd = fopen(file, "r+b");


          for (i = options.securedelete; i > 0; i--) {
            fseek(fd, 0, SEEK_SET);
       
            for (j = 0; j < k; j += lsize) {
              g = random();
              memcpy(garbage, &g, lsize);
              fwrite(garbage, lsize, 1, fd);
            }
            fflush(fd);
          }
          fclose(fd);
        }

        if (unlink(file)) {
          fprintf(stderr, "Error deleting file %s\n", file);
          return(1);
        }
        return(0);
      }

      /////////////// BFwrapzl      

      uLong docompress(char **input, uLong sz) {
        uLong newsz;
        char *output;

        newsz = sz + (sz *.1) + 13;
        if ((output = (char *)malloc(newsz + 1)) == NULL)
          memerror();

        memset(output, 0, newsz + 1);

        compress((Bytef *) output, &newsz, (const Bytef *) *input,  sz);

        free(*input);
        if ((*input = (char *)malloc(newsz)) == NULL)
          memerror();

        memcpy(*input, output, newsz);

        free(output);

        return(newsz);
      }

      uLong douncompress(char **input, uLong sz, BCoptions options) {
        char *output;

        if ((output = (char *)malloc(options.origsize + 1)) == NULL)
          memerror();

        memset(output, 0, options.origsize + 1);

        uncompress((Bytef *) output, (uLong *) &options.origsize, 
        (const Bytef *) *input, sz);

        free(*input);
        if ((*input = (char *)malloc(options.origsize)) == NULL)
          memerror();

        memcpy(*input, output, options.origsize);

        free(output);

        return(options.origsize);
      }      

      /////////////// Bfendian      
   
      void getEndian(unsigned char **e) {
        short i = 0x4321;
        int bigE = (*(char*) &i);

        if ((*e = (unsigned char *)realloc(*e, sizeof(char) + 1)) == NULL)
          memerror();

        memset(*e, 0, sizeof(char) + 1);

        if (bigE == 0x43)
          memset(*e, endianBig, 1);
        else if (bigE == 0x21)
          memset(*e, endianLittle, 1);
      }

      uInt32 swapEndian(uInt32 value) {
        unsigned int b1, b2, b3, b4;
        uInt32 swapped;

        b4 = (value>>24) & 0xff;
        b3 = (value>>16) & 0xff;
        b2 = (value>>8) & 0xff;
        b1 = value & 0xff;

        swapped = (b1<<24) | (b2<<16) | (b3<<8) | b4;
        return(swapped);
      }

      int testEndian(char *input) {
        unsigned char *myEndian = NULL, *yourEndian = NULL;

        getEndian(&myEndian);
        if ((yourEndian = (unsigned char *)malloc(2)) == NULL)
          memerror();

        memset(yourEndian, 0, 2);
        memcpy(yourEndian, input, 1);

        if (memcmp(myEndian, yourEndian, 1) == 0)
          return(0);

        return(1);
      }

      int swapCompressed(char **input, uLong sz) {
        char c;
        unsigned int j, swap;

        memcpy(&c, *input+1, 1);

        if (c == 0)
          return(0);

        j = sizeof(uInt32);

        memcpy(&swap, *input+(sz - j), j);
        swap = swapEndian(swap);
        memcpy(*input+(sz - j), &swap, j);

        return(1);
      }
  };

}
#endif

#if 0
extern char *optarg;
extern int optind, optreset, opterr;

int usage(char *name) {
  fprintf(stderr, "Usage is: %s -[orc][-sN] file1 file2..\n", name);
  if (STANDARDOUT == 0)
    fprintf(stderr, "  -o Write output to standard out\n");
  else
    fprintf(stderr, "  -o Don't write output to standard out\n");
  if (REMOVE == 1)
    fprintf(stderr, "  -r Do NOT remove input files after processing\n");
  else
    fprintf(stderr, "  -r Remove input files after processing\n");
  if (COMPRESS == 1)
    fprintf(stderr, "  -c Do NOT compress files before encryption\n");
  else
    fprintf(stderr, "  -c Compress files before encryption\n");

  fprintf(stderr, "  -sN How many times to overwrite input files with random\
 data\n");

  exit(1);
}

int memerror() {
  fprintf(stderr, "Can't allocate enough memory. Bailing out\n");
  exit(1);
}

int parseArgs(int *argc, char **argv, BCoptions *options) {
  signed char ch; 
  char *progname;

  if ((progname = malloc(strlen(argv[0]) + 1)) == NULL)
    memerror();
  memcpy(progname, argv[0], strlen(argv[0]) + 1);

  *options = initoptions(*options);
    
  while ((ch = getopt(*argc, argv, "rocs:")) != -1) {
    
    switch(ch) {
      case 'r':
      /* remove files after write (default on) */
	if (options->remove == 1)
          options->remove = 0;
	else
	  options->remove = 1;

        break;
     
      case 'o':
      /* write to stdout */
	if (options->standardout == 0)
          options->standardout = 1;
	else
	  options->standardout = 0;

	options->remove = 0;
        break;
     
      case 'c':
      /* compress files (default on) */
	if (options->compression == 1)
          options->compression = 0;
	else
	  options->compression = 1;
        break;

      case 's':
      /* how many times to overwrite data (default 3) */
	options->securedelete = atoi(optarg);
	break;

      default:
        /* dump the usage message */
        usage(progname);
    }
  }
      
  *argc -= optind;
    
  if (*argc < 1) {
    usage(progname);
  }

  return(optind);
}

int assignFiles(char *arg, char **infile, char **outfile, struct stat *statbuf,
	BCoptions *options, char *key) {

  if (lstat(arg, statbuf) < 0)
    return(1);
  if (!S_ISREG(statbuf->st_mode))
    return(1);

  if ((*infile = realloc(*infile, strlen(arg) + 1)) == NULL)
    memerror();
  memset(*infile, 0, strlen(arg) + 1);
  strcpy(*infile, arg);

  if ((*outfile = realloc(*outfile, strlen(arg) + 5)) == NULL)
    memerror();
  memset(*outfile, 0, strlen(arg) + 5);
  strcpy(*outfile, *infile);

  if (strlen(*infile) > 4) {

    if ((memcmp(*infile+(strlen(*infile) - 4), ".bfe", 4) == 0) &&
              ((!key) || (options->type == DECRYPT))){

      memset(*outfile+(strlen(*infile) - 4), 0, 4);
      options->type = DECRYPT;

    } else if ((!key) || (options->type == ENCRYPT)){
      if (memcmp(*infile+(strlen(*infile) - 4), ".bfe", 4) == 0)
        return(1);

      strcat(*outfile, ".bfe");
      options->type = ENCRYPT;

    } else
      return(1);

  } else if ((!key) || (options->type == ENCRYPT)) {
    strcat(*outfile, ".bfe");
    options->type = ENCRYPT;
  } else
    return(1);
  return(0);
}

int main(int argc, char *argv[]) {
  uLong sz = 0;
  char *input = NULL, *key = NULL, *key2 = NULL; 
  char *infile = NULL, *outfile = NULL;
  struct stat statbuf;
  BCoptions options;

  argv += parseArgs(&argc, argv, &options);

  for (; argc > 0; argc--, argv++) {

    if (assignFiles(argv[0], &infile, &outfile, &statbuf, &options, key))
      continue;

    if (!key) {
      key = getkey(options.type);
      mutateKey(&key, &key2);
    }

    sz = readfile(infile, &input, options.type, key, statbuf);

    if ((options.type == DECRYPT) && (testEndian(input))) 
      swapCompressed(&input, sz);

    if ((options.compression == 1) && (options.type == ENCRYPT)) {
      options.origsize = sz;
      sz = docompress(&input, sz);
    }

    if (options.type == ENCRYPT) {
      sz = attachKey(&input, key, sz);
      sz = padInput(&input, sz);
    }

    if (options.type == ENCRYPT) 
      sz = BFEncrypt(&input, key, sz, &options); 
    else if (options.type == DECRYPT)
      if ((sz = BFDecrypt(&input, key, key2, sz, &options)) == 0) {
        fprintf(stderr, "Invalid encryption key for file: %s\n", infile);
        exit(1);
      }

    if ((options.compression == 1) && (options.type == DECRYPT)) 
      sz = douncompress(&input, sz, options);

    writefile(outfile, input, sz, options, statbuf);

    if ((input = realloc(input, sizeof(char *))) == NULL)
      memerror();

    if (options.remove == 1) 
      deletefile(infile, options, key, statbuf);

  if (input != NULL)
    free(input);
  }

  if(!sz)
    fprintf(stderr, "No valid files found\n");

  return(0);
}
#endif
