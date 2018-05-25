#include <stdio.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/wait.h>

  int main() {

    int stdin_copy = dup(fileno(stdin));
    int stdout_copy = dup(fileno(stdout));
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    while (1) {
      dup2(stdin_copy, fileno(stdin));
      dup2(stdout_copy, fileno(stdout));

      char command[100];// = (char * ) malloc(100);
      char str[100+1];// = (char * ) malloc(100);
      char strNew[100];// = (char * ) malloc(100);
      char strNew2[100];// = (char * ) malloc(100);
      char temp2[100];// = (char * ) malloc(100);
	memset(str, 0, 100);
	memset(strNew, 0, 100);
	memset(strNew2, 0, 100);
	memset(temp2, 0, 100);
	memset(command, 0, 100);
      char * pch;
      char * token;
      size_t sz = 100;
      int i = 0;
      int j = 0;
      int index = 0;
      int tempIndex = 0;
      int ii = 0;
      int jj = 0;
      pid_t pid;

      //find path

      char * envPath = (char * ) malloc(strlen(getenv("PATH"))+1);
      char * tempAddr = (char * ) malloc(strlen(getenv("PATH"))+1);
      strcpy(envPath, getenv("PATH"));
      strcpy(tempAddr, envPath);
      token = strtok(envPath, ":");

      while (token != NULL) {
        token = strtok(NULL, ":");
        index++;
      }
      char * addr[index + 1];
      token = strtok(tempAddr, ":");

      while (token != NULL) {
        addr[tempIndex] = (char * ) malloc(strlen(token)+1);
        strcpy(addr[tempIndex], token);
        token = strtok(NULL, ":");
        tempIndex++;
      }
      addr[index] = 0;
	//printf("index=%d, tempindex=%d", index, tempIndex);
      //parse command
      printf("myShell:\n");
	if (fgets(str, sz+1, stdin)==NULL){
		printf("sa;am");
		//printf("%s",str);
	return 1;
		//strcpy(str,"");
		//printf("YY");
	}
      
		
	//strncpy(strNew2, str, strlen(str)-1);
	//strcpy(strNew2, str);
strncpy(strNew2,str,strlen(str)-1);

      //extract pip
      strcpy(temp2, strNew2);
      pch = strtok(strNew2, "|");
      while (pch != NULL) {
        pch = strtok(NULL, " ");
        ii++;
      }


      char * section[ii];
      pch = strtok(temp2, "|");
      while (pch != NULL) {
        section[jj] = (char * ) malloc(strlen(pch));
        strcpy(section[jj], pch);
        pch = strtok(NULL, "|");

        jj++;
      }
      int numPipes = ii - 1;
      int pipefds[2 * numPipes];
      for (i = 0; i < (numPipes); i++) {
        if (pipe(pipefds + i * 2) < 0) {
          perror("couldn't pipe");
          exit(EXIT_FAILURE);
        }
      }

      //***

      char * temp[jj];

      for (int k = 0; k < jj; k++) {
        temp[k] = (char * ) malloc(strlen(section[k]));
        strcpy(temp[k], section[k]);
        char newString[200][200];
	memset(newString,0,200*200);
        char token;
        int ctr = 0;
        int s = 0;
        //
        for (int m = 0; m <= (strlen(section[k])); m++) {
          if (section[k][m] == '"') {
            int l = 1;

            token = section[k][m + l];
            while (token != '"') {
              newString[ctr][s] = token;
              s++;
              l++;
              token = section[k][m + l];
            }
            m = m + l;
          }

          if ((section[k][m] == ' ' || section[k][m] == '\0' || section[k][m] == '\n') && s != 0) {
            newString[ctr][s] = '\0';
            ctr++; //for next word
            s = 0; //for next word, init index to 0
          } else if ((section[k][m] != ' ' && section[k][m] != '\0' && section[k][m] != '"' && section[k][m] != '\n' )) {
            newString[ctr][s] = section[k][m];
            s++;
          }
        }
        char * argv[jj][ctr + 1];
        int q = 0;
        for (int h = 0; h < ctr; h++) {

          argv[k][h] = (char * ) malloc(strlen(newString[h]));
          if (newString[h][0] != '\0') {
            strcpy(argv[k][h], newString[h]);
	//printf("-----%s\n",newString[h]);
            q++;
          }
        }

        argv[k][ctr] = 0;
        //

        //make final address

        char finalAdd[200];// = (char * ) malloc(100);
	memset(finalAdd, 0 , 200);
        for (int s = 0; s < tempIndex; s++) {

          strcpy(finalAdd, addr[s]);
          strcat(finalAdd, "/");
          strcat(finalAdd, argv[k][0]);

          if (access(finalAdd, X_OK) == 0)
            break;

        }
	if (strcmp(argv[k][0], "exit") == 0)
	{
		//printf("ee%s",cwd);
		exit(0);
	}
        else if (strcmp(argv[k][0], "cd") == 0)
	{
            if(chdir(argv[k][1])!=0){
		printf(" %s - path not found\n",argv[k][1]);}
			
	}
	else{

        pid_t pid = fork();

        if (pid == 0) {

          // child

          //if not first command
          if (k != 0) {
            if (dup2(pipefds[(k - 1) * 2], 0) < 0) {
              perror(" dup2"); ///j-2 0 j+1 1
              exit(EXIT_FAILURE);

            }
            close(pipefds[(k - 1) * 2]);
            close(pipefds[(k - 1) * 2 + 1]);
          }
          //if not last command
          if (k != (jj - 1)) {
            close(pipefds[k * 2]);
            if (dup2(pipefds[k * 2 + 1], 1) < 0) {
              perror("dup2");
              exit(EXIT_FAILURE);
            }
            close(pipefds[k * 2 + 1]);
          }


        if (execv(finalAdd, argv[k])) {
            printf("err%d%s\n", errno, strerror(errno));
            return 254;
          }
        } else {
          if (k != 0)
            close(pipefds[(k - 1) * 2]);
          close(pipefds[(k - 1) * 2 + 1]);
          if (k != (jj - 1))
            dup2(pipefds[(k - 1) * 2 + 1], pipefds[(k) * 2]);

          // Wait for child process to finish
          int childStatus;
          waitpid(pid, & childStatus, 0);

        }


      }//else

}//for
	free (envPath);
	free(tempAddr);
	for(int s=0;s< (index+1);s++)
		free(addr[s]);
	for(int s=0;s<ii;s++){
		free(section[s]);
		}
	for(int s=0;s<jj;s++){
		free(temp[s]);
		}


    }//while
    return 0;
  }
