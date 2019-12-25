#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ftree.h"
#include "hash.h"

/*
 * Helper function.
 * Returns the full path of a subdirectory or a file.
 */
char *build_path(const char *root, char *filename){
	char *path = malloc(strlen(root) + strlen(filename) + 2);
	// Properly build the path.
	strcpy(path, root);
	strcat(path, "/");
	strcat(path, filename);
	strcat(path, "\0");
	return path;
}

/*
 * Helper function.
 * Returns the file name from an absolute path.
 */
char *get_fname(char *path){
	char *filename;
	// Basename returns the name of the file after the last "/" in path.
	// If the path does not have any slashes, then a copy of path.
	filename = basename(path);
	return filename;
}

/*
 * Returns the FTree rooted at the path fname.
 */
struct TreeNode *generate_ftree(const char *fname) {
    // Your implementation here.
    struct TreeNode *root = (struct TreeNode*) malloc(sizeof(struct TreeNode));
    struct stat s;
	if (lstat(fname, &s) < 0){
		perror("lstat");
		exit(EXIT_FAILURE);
	}
	// If fname is a regular file or link
	if (S_ISREG(s.st_mode) || S_ISLNK(s.st_mode)) {
		root->fname = get_fname((char *) fname);
		root->permissions = s.st_mode & 0777;
		root->contents = NULL;
		root->next = NULL;
		FILE *f; 
		f = fopen(fname, "r");
		if (f == NULL){
			// In case the file did not successfully open, then simply set its hash to zeros
			// since we did not read any contents.
			perror("Could not open file");
			root->hash = (char *) malloc(BLOCK_SIZE * sizeof(char));
			for (int i = 0; i < BLOCK_SIZE; i++){
				root->hash[i] = '0';
			}
		} else {
			root->hash = hash(f);
			fclose(f);
		}
	} else { // fname is a directory
		DIR *dir;
		struct dirent *direntry;
		// The previous node (first call has no previous)
		struct TreeNode *prev = NULL;
		root->fname = get_fname((char *)fname);
		root->permissions = s.st_mode & 0777;
		root->hash = NULL;
		root->contents = NULL;
		root->next = NULL;
		dir = opendir(fname);
		if (dir == NULL){
			perror("Could not open directory");
		}
		while ( (direntry = readdir(dir)) != NULL){
			if ((*direntry).d_name[0] != '.'){
				// Current directory entry
				struct TreeNode *curr = (struct TreeNode*) malloc(sizeof(struct TreeNode));
				// Using the helper function to build the path for subdirs and files
				char *path = build_path(fname, (*direntry).d_name);
				curr = generate_ftree(path);
				// In the first call, there is no previous so we set one
				if (prev == NULL){
					prev = curr;
				} else {
				// After the first call, we move the pointer to create a LinkedList
					prev->next = curr;
					prev = prev->next;
				}
				// In the first call, the root's contents is null so we point the first directory entry to it.
				if (root->contents == NULL){
					root->contents = prev;
				}
			}
		}
		closedir(dir);
	}
	return root;
}

/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 */
void print_ftree(struct TreeNode *root) {
    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.
    static int depth = 0;
    printf("%*s", depth * 2, "");
	// To check if its a file
	if (root->contents == NULL && root->hash != NULL){
		printf("%s (%o)\n", root->fname, root->permissions);
	} else if (root->contents == NULL && root->hash == NULL){ // Or an empty directory
		printf("===== %s (%o) =====\n", root->fname, root->permissions);
	} else { // Otherwise, it is a non-empty directory..
		printf("===== %s (%o) =====\n", root->fname, root->permissions);
		// Increment the depth before we recurse.
		depth++;
		print_ftree(root->contents);
		// Decrement the depth so that if there are files left in the main directory
		// they are displayed properly.
		depth--;
		}
	// To print the next file
	if (root->next != NULL){
		print_ftree(root->next);	
	}
}

