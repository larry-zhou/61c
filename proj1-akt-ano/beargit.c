#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - Here are some of the helper functions from util.h:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the project spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);

  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");
  return 0;
}



/* beargit add <filename>
 *
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR:  File <filename> has already been added.
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR:  File %s has already been added.\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the project spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  fprintf(stdout, "%s\n\n","Tracked files:");
  int fileCount = 0;
  FILE *index = fopen(".beargit/.index","r");
  char str[FILENAME_SIZE];

  while (fgets(str, sizeof(str), index)) {
    fprintf(stdout, "%s", str);
    fileCount++;
  }

  fprintf(stdout, "\n%s%d%s\n", "There are ", fileCount, " files total."); //\n%s%d%s??
  fclose(index);
  return 0;
}

/* beargit rm <filename>
 *
 * See "Step 2" in the project spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  FILE *index = fopen(".beargit/.index", "r");
  FILE *newindex = fopen(".beargit/.newindex", "w");
  char str[FILENAME_SIZE];
  int tracked = 0;

  while (fgets(str, sizeof(str), index)) {
    strtok(str, "\n");
    if (strcmp(str, filename) != 0) { //if str is not filename, copy it
      fprintf(newindex, "%s\n", str);
    } else {
      tracked = 1;
    }
  }

  fclose(index);
  fclose(newindex);

  if (tracked) {
    fs_mv(".beargit/.newindex", ".beargit/.index");
    return 0;
  } else {
    fprintf(stderr, "ERROR:  File %s not tracked.\n", filename); //untracked file, error
    return 1;
  }
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the project spec.
 *
 */

const char* go_bears = "THIS IS BEAR TERRITORY!";

int is_commit_msg_ok(const char* msg) { //returns 1 if okay, returns 0 if not okay
  /* COMPLETE THE REST */
  int i = 0;
  int j = 0;
  while (go_bears[i] != '\0' && msg[j] != '\0') {
    // if (i == 23) { //length of go_bears
    //   return 1;}
    if (go_bears[i] != msg[j]) {
      i = 0;
    }
    i++;
    j++;
    if (i == 23) { //length of go_bears
      return 1;
    } 
  }
  return 0;
}

/* Use next_commit_id to fill in the rest of the commit ID.
 *
 * Hints:
 * You will need a destination string buffer to hold your next_commit_id, before you copy it back to commit_id
 * You will need to use a function we have provided for you.
 */

void next_commit_id(char* commit_id) {
  /* COMPLETE THE REST */
  //MUST HASH THE BRANCH IN
  //SHA_HEX_BYTES = 40
  //SHA_DIGEST_LENGTH =20
  //COMMIT ID LENGTH = 40
  // void cryptohash(const char* str, char dst[SHA_HEX_BYTES + 1]) {
  //      char buf[SHA_DIGEST_LENGTH];
  //      SHA1((unsigned char*)str, strlen(str), (unsigned char*) buf);
  //      for (size_t i = 0; i < SHA_DIGEST_LENGTH; ++i) {
  //           sprintf(&dst[i*2], "%02x", (unsigned char)buf[i]); // %02x means convert it into x and fill it with 0s if the hex is one digit long
  //      }
  //      dst[SHA_HEX_BYTES] = '\0';
  // }
  char* hashed = commit_id;
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);
  int i = 0;
  while (current_branch[i] != '\0') {
    hashed[i] = current_branch[i];
    i++;
  }
  cryptohash(commit_id, hashed);
  commit_id = hashed;
}

int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR:  Message must contain \"%s\"\n", go_bears);
    return 1;
  }
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);
  if (strlen(current_branch) == 0) {
    fprintf(stderr, "ERROR: Need to be on HEAD of a branch to commit\n");
    return 1;
  }
  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);
  //do not edit anything above this line.
  /* COMPLETE THE REST */
  char new_commit_dir[FILENAME_SIZE];
  char commit_index[FILENAME_SIZE];
  char prev_directory[FILENAME_SIZE];
  char file_name[FILENAME_SIZE];
  sprintf(new_commit_dir, "%s%s", ".beargit/", commit_id);
  fs_mkdir(new_commit_dir);
  sprintf(commit_index, "%s/.index", new_commit_dir);
  fs_cp(".beargit/.index", commit_index);
  sprintf(prev_directory, "%s/.prev", new_commit_dir);
  fs_cp(".beargit/.prev", prev_directory);
  FILE* index = fopen(".beargit/.index", "r");
  while (fgets(file_name, sizeof(file_name), index)) {
    char current_file_dir[FILENAME_SIZE];
    strtok(file_name, "\n");
    sprintf(current_file_dir, "%s/%s", new_commit_dir, file_name);
    fs_cp(file_name, current_file_dir);
  }
  char curr_msg[FILENAME_SIZE];
  sprintf(curr_msg, "%s/.msg", new_commit_dir);
  write_string_to_file(curr_msg, msg);
  write_string_to_file(".beargit/.prev", commit_id);
  fclose(index);
  return 0;
}

/* beargit log
 *
 * See "Step 4" in the project spec.
 *
 */

int beargit_log(int limit) {
  // /* COMPLETE THE REST */
  int no_commit = 1; //boolean to see if it went into the while loop
  int firstline = 1; // boolean to see if it is the first line so that it doesnt print a new line
  char curr_id[COMMIT_ID_SIZE]; //initialize current id which points to the current commit id
  read_string_from_file(".beargit/.prev", curr_id, COMMIT_ID_SIZE);
  while (strcmp(curr_id, "0000000000000000000000000000000000000000")) { // loop while not on the newest commit
    no_commit = 0;
    char curr_msg[MSG_SIZE];
    char prev_directory[FILENAME_SIZE];
    if (firstline) { //firstline deals with the new line that is before every output
      firstline = 0;
    } else {
      fprintf(stdout, "\n");
    }
    fprintf(stdout, "commit %s", curr_id);
    sprintf(curr_msg, "%s%s%s", ".beargit/", curr_id, "/.msg");
    read_string_from_file(curr_msg, curr_msg, MSG_SIZE);
    fprintf(stdout, "\n   %s\n", curr_msg);
    sprintf(prev_directory, "%s%s%s", ".beargit/", curr_id,  "/.prev"); //writing prev_directory
    read_string_from_file(prev_directory, curr_id, COMMIT_ID_SIZE);
  }
  if (no_commit) {
    fprintf(stderr, "ERROR: There are no commits!\n");
    return 1;
  }
  if (!no_commit)
    fprintf(stdout, "\n");
  return 0;
}


// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int counter = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = counter;
    }
    counter++;
  }

  fclose(fbranches);

  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the project spec.
 *
 */

 
 int beargit_branch() {
   /* COMPLETE THE REST */
  FILE *branches = fopen(".beargit/.branches","r");
  char curr_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", curr_branch, BRANCHNAME_SIZE);
  char branchname[BRANCHNAME_SIZE];
  while (fgets(branchname, sizeof(branchname), branches)) {
    strtok(branchname, "\n");
    if (strcmp(branchname, curr_branch) == 0) { //if branch being printed is curr branch put a * in front. strmp = 0 if strings are the same.
      fprintf(stdout, "*  %s\n", branchname);
    } else {
      fprintf(stdout, "   %s\n", branchname);
    }
  }
  fclose(branches);
  return 0;
 }

/* beargit checkout
 *
 * See "Step 6" in the project spec.
 *
 */

int checkout_commit(const char* commit_id) {
  /* COMPLETE THE REST */
  if (!strcmp(commit_id, "0000000000000000000000000000000000000000")) {
    write_string_to_file(".beargit/.index", "");
  } else {
    FILE* findex = fopen(".beargit/.index", "r");
    char line[FILENAME_SIZE];
    while(fgets(line, sizeof(line), findex)) {
      strtok(line, "\n");
      fs_rm(line);
    }
    fclose(findex);
    char commit_index[FILENAME_SIZE];
    sprintf(commit_index, ".beargit/%s/.index", commit_id);
    FILE* fcommit_index = fopen(commit_index, "r");
    char new_line[FILENAME_SIZE];
    while(fgets(new_line, sizeof(new_line), fcommit_index)){
      strtok(new_line, "\n");
      char new_dest[FILENAME_SIZE];
      sprintf(new_dest, ".beargit/%s/%s", commit_id, new_line);
      fs_cp(new_dest, new_line);
    }
  fs_cp(commit_index, ".beargit/.index");
  fclose(fcommit_index);
  }
  write_string_to_file(".beargit/.prev", commit_id);
  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  /* COMPLETE THE REST */
  char dir[FILENAME_SIZE];
  sprintf(dir, ".beargit/%s", commit_id);
  return fs_check_dir_exists(dir);
}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);
  // If not detached, update the current branch by storing the current HEAD into that branch's file...
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE+50];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

  // Check whether the argument is a commit ID. If yes, we just stay in detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    char commit_dir[FILENAME_SIZE] = ".beargit/";
    strcat(commit_dir, arg);
    if (!fs_check_dir_exists(commit_dir)) {
      fprintf(stderr, "ERROR:  Commit %s does not exist.\n", arg);
      return 1;
    }

    // Set the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }

  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(branch_name) >= 0);

  // Check for errors.
  if (branch_exists && new_branch) {
    fprintf(stderr, "ERROR:  A branch named %s already exists.\n", branch_name);
    return 1;
  } else if (!branch_exists && !new_branch) {
    fprintf(stderr, "ERROR: No branch or commit %s exists\n", branch_name);
    return 1;
  }

  // File for the branch we are changing into.
  char branch_file[FILENAME_SIZE] = ".beargit/.branch_";
  strcat(branch_file, branch_name);

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file);
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);

  // Check out the actual commit.

  return checkout_commit(branch_head_commit_id);
}

/* beargit reset
 *
 * See "Step 7" in the project spec.
 *
 */

int beargit_reset(const char* commit_id, const char* filename) {
  if (!is_it_a_commit_id(commit_id)) {
      fprintf(stderr, "ERROR:  Commit %s does not exist.\n", commit_id);
      return 1;
  }

  // Check if the file is in the commit directory
  /* COMPLETE THIS PART */
  char commit_index[FILENAME_SIZE];
  sprintf(commit_index, ".beargit/%s/.index", commit_id);
  FILE* fcommit_index = fopen(commit_index, "r");
  char new_line[FILENAME_SIZE];
  int exists = 0;
  while(fgets(new_line, sizeof(new_line), fcommit_index)){
    strtok(new_line, "\n");
    if (strcmp(new_line, filename) == 0) { //if new_line is filename, do nothing
      exists = 1;
    }
  }
  if (!exists) {
    fprintf(stderr, "ERROR:  %s is not in the index of commit %s.\n",filename, commit_id);
  }
  fclose(fcommit_index);
  // // Copy the file to the current working directory
  // /* COMPLETE THIS PART */
  char file_path[FILENAME_SIZE];
  sprintf(file_path, ".beargit/%s/%s", commit_id, filename);
  char file_dest[FILENAME_SIZE];
  sprintf(file_dest, "./%s", filename);
  fs_cp(file_path, file_dest);
  // Add the file if it wasn't already there
  /* COMPLETE THIS PART */
  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fclose(findex);
      return 0;
    }
  }
  beargit_add(filename);
  return 0;
}

/* beargit merge
 *
 * See "Step 8" in the project spec.
 *
 */

int beargit_merge(const char* arg) {
  // Get the commit_id or throw an error
  char commit_id[COMMIT_ID_SIZE];
  if (!is_it_a_commit_id(arg)) {
      if (get_branch_number(arg) == -1) {
            fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
            return 1;
      }
      char branch_file[FILENAME_SIZE];
      snprintf(branch_file, FILENAME_SIZE, ".beargit/.branch_%s", arg);
      read_string_from_file(branch_file, commit_id, COMMIT_ID_SIZE);
  } else {
      snprintf(commit_id, COMMIT_ID_SIZE, "%s", arg);
  }

  // Iterate through each line of the commit_id index and determine how you
  // should copy the index file over
   /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  char commit_index[FILENAME_SIZE];
  sprintf(commit_index, ".beargit/%s/.index", commit_id);
  FILE* fcommit_index = fopen(commit_index, "r");
  char new_line[FILENAME_SIZE];
  int exists = 0;  
  while(fgets(new_line, sizeof(new_line), fcommit_index)){
    strtok(new_line, "\n");
    while(fgets(line, sizeof(line), findex)) {
      strtok(line, "\n");
      if (strcmp(new_line, line) == 0) { //if new_line is filename, do nothing
        exists = 1;
        break;
      }          
    }
    if (exists) {
      char conf_file_src[FILENAME_SIZE];
      char conf_file_dest[FILENAME_SIZE];
      sprintf(conf_file_src, ".beargit/%s/%s", commit_id, new_line);
      sprintf(conf_file_dest, "./%s.%s", new_line, commit_id);
      fs_cp(conf_file_src, conf_file_dest);
      fprintf(stdout, "%s conflicted copy created\n", new_line);
      exists = 0;
    } else {
      beargit_add(new_line);
      char file_src[FILENAME_SIZE];
      char file_dest[FILENAME_SIZE];
      sprintf(file_src, ".beargit/%s/%s", commit_id, new_line);
      sprintf(file_dest, "./%s", new_line);
      fs_cp(file_src, file_dest);
      fprintf(stdout, "%s added\n", new_line);
    }
  }
  fclose(findex);
  fclose(fcommit_index);  
  return 0;
}
