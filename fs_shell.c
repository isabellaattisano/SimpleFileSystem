
#include "fs.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

void do_format(Disk *disk, FileSystem *fs, int args, char *arg);
void do_mount(Disk *disk, FileSystem *fs, int args, char *arg);
void do_create(Disk *disk, FileSystem *fs, int args, char *arg);
void do_remove(Disk *disk, FileSystem *fs, int args, char *arg);
void do_cat(Disk *disk, FileSystem *fs, int args, char *arg);
void do_write(Disk *disk, FileSystem *fs, int args, char *arg);
void do_help();


int main(int argc, char *argv[]){

    if (argc != 3) {
        return -1;
    }

    Disk *disk = disk_open(argv[1], atoi(argv[2]));
    FileSystem fs;

    int MAX = 100;
    char cmd[MAX], arg[MAX], line[MAX];

    printf("\n\033[1;35mfilesystem$ \033[0;36m");
    fgets(line, MAX, stdin);
    int args = sscanf(line, "%s %s", cmd, arg);
    write(1, "\033[0;30m", strlen("\033[0;30m"));

    while(strncmp(cmd, "quit", 4) !=0){
        if (strcmp(cmd, "format")==0) {
	        do_format(disk, &fs, args, arg);
        } else if (strcmp(cmd, "mount")==0) {
	        do_mount(disk, &fs, args, arg);
        } else if (strcmp(cmd, "create")==0) {
	        do_create(disk, &fs, args, arg);
        } else if (strcmp(cmd, "remove")==0) {
	        do_remove(disk, &fs, args, arg);
        } else if (strcmp(cmd, "cat")==0) {
	        do_cat(disk, &fs, args, arg);
        } else if (strcmp(cmd, "help")==0) {
	        do_help();
        } else if (strcmp(cmd, "write")==0) {
	        do_write(disk, &fs, args, arg);
        } else {
	         printf("Unknown command: %s\n", cmd);
	         printf("Type 'help' for a list of commands.\n");
	    }

         printf("\n\033[1;35mfilesystem$ \033[0;36m");
         fgets(line, MAX, stdin);
         args = sscanf(line, "%s %s", cmd, arg);
         write(1, "\033[0;30m", strlen("\033[0;30m"));

    }
     fs_unmount(&fs);
     disk_close(disk);
     exit(0);
}

void do_format(Disk *disk, FileSystem *fs, int args, char *arg){

    if (fs_format(disk)) {
        printf("Disk formatted\n");
        
    } else {
        printf("Format failed!\n");
    }

}
void do_mount(Disk *disk, FileSystem *fs, int args, char *arg){

    if (fs_mount(fs, disk)) {
        printf("Disk mounted\n");
    } else {
        printf("Mount failed!\n");
    }
}

void do_create(Disk *disk, FileSystem *fs, int args, char *arg){
    if(args < 2){
        printf("create <file name>\n");
        return;
    }
    char *str = (char*)malloc(sizeof(char*));
    strcpy(str, arg);

    ssize_t inode_number = fs_create(fs, str);
    
    if(inode_number>-1){
        printf("created %s with inode number %ld\n", str, inode_number);
    }
    else{
        printf("Could not create file: %s\n", arg);
    }

}

void do_remove(Disk *disk, FileSystem *fs, int args, char *arg){

     if(args < 2){
        printf("remove <file name>\n");
        return;
    }

    if(fs_remove(fs, arg)){
        printf("removed file: %s\n", arg);
    }
    else
        printf("could not remove: %s\n", arg);


}

void do_cat(Disk *disk, FileSystem *fs, int args, char *arg){
    if(args < 2){
        printf("cat <file name>\n");
        return;
    }
    char cat[BLOCK_SIZE];

    ssize_t ret= fs_read(fs, arg, 1, cat, fs_stat(fs, arg), 0);
    if(ret > 0)
            printf("\n\nRead completed. Read %ld bytes.\n", ret);
       else if(ret ==0)
            printf("\n\nRead completed. No data was read.\n");
        else 
             printf("\n\nRead Failed.\n");
}

void do_write(Disk *disk, FileSystem *fs, int args, char *arg){

    if(args < 2){
        printf("write <file name>\n");
        return;
    }

    // char g[BLOCK_SIZE*2] = "Representatives and direct Taxes shall be apportioned among the several States which may be included within this Union, according to their respective Numbers, which shall be determined by adding to the whole Number of free Persons, including those bound to Service for a Term of Years, and excluding Indians not taxed, three fifths of all other Persons. The actual Enumeration shall be made within three Years after the first Meeting of the Congress of the United States, and within every subsequent Term of ten Years, in such Manner as they shall by Law direct. The Number of Representatives shall not exceed one for every thirty Thousand, but each State shall have at Least one Representative; and until such enumeration shall be made, the State of New Hampshire shall be entitled to chuse three, Massachusetts eight, Rhode-Island and Providence Plantations one, Connecticut five, New-York six, New Jersey four, Pennsylvania eight, Delaware one, Maryland six, Virginia ten, North Carolina five, South Carolina five, and Georgia three.This Constitution, and the Laws of the United States which shall be made in Pursuance thereof; and all Treaties made, or which shall be made, under the Authority of the United States, shall be the supreme Law of the Land; and the Judges in every State shall be bound thereby, any Thing in the Constitution or Laws of any State to the Contrary notwithstanding.The Senators and Representatives before mentioned, and the Members of the several State Legislatures, and all executive and judicial Officers, both of the United States and of the several States, shall be bound by Oath or Affirmation, to support this Constitution; but no religious Test shall ever be required as a Qualification to any Office or public Trust under the United StatesThe judicial Power shall extend to all Cases, in Law and Equity, arising under this Constitution, the Laws of the United States, and Treaties made, or which shall be made, under their Authority;--to all Cases affecting Ambassadors, other public Ministers and Consuls;--to all Cases of admiralty and maritime Jurisdiction;--to Controversies to which the United States shall be a Party;--to Controversies between two or more States;-- between a State and Citizens of another State;--between Citizens of different States;--between Citizens of the same State claiming Lands under Grants of different States, and between a State, or the Citizens thereof, and foreign States, Citizens or Subjects. all Cases affecting Ambassadors, other public Ministers and Consuls, and those in which a State shall be Party, the supreme Court shall have original Jurisdiction. In all the other Cases before mentioned, the supreme Court shall have appellate Jurisdiction, both as to Law and Fact, with such Exceptions, and under such Regulations as the Congress shall make.The Trial of all Crimes, except in Cases of Impeachment, shall be by Jury; and such Trial shall be held in the State where the said Crimes shall have been committed; but when not committed within any State, the Trial shall be at such Place or Places as the Congress may by Law have directed.Treason against the United States, shall consist only in levying War against them, or in adhering to their Enemies, giving them Aid and Comfort. No Person shall be convicted of Treason unless on the Testimony of two Witnesses to the same overt Act, or on Confession in open Court.The Congress shall have Power to declare the Punishment of Treason, but no Attainder of Treason shall work Corruption of Blood, or Forfeiture except during the Life of the Person attainted.ull Faith and Credit shall be given in each State to the public Acts, Records, and judicial Proceedings of every other State. And the Congress may by general Laws prescribe the Manner in which such Acts, Records and Proceedings shall be proved, and the Effect thereof.A Person charged in any State with Treason, Felony, or other Crime, who shall flee from Justice, and be found in another State, shall on Demand of the executive Authority of the State from which he fled, be delivered up, to be removed to the State having Jurisdiction of the Crime.No Person held to Service or Labour in one State, under the Laws thereof, escaping into another, shall, in Consequence of any Law or Regulation therein, be discharged from such Service or Labour, but shall be delivered up on Claim of the Party to whom such Service or Labour may be due.Section. 3.New States may be admitted by the Congress into this Union; but no new State shall be formed or erected within the Jurisdiction of any other State; nor any State be formed by the Junction of two or more States, or Parts of States, without the Consent of the Legislatures of the States concerned as well as of the Congress.The Congress shall have Power to dispose of and make all needful Rules and Regulations respecting the Territory or other Property belonging to the United States; and nothing in this Constitution shall be so construed as to Prejudice any Claims of the United States, or of any particular State.The United States shall guarantee to every State in this Union a Republican Form of Government, and shall protect each of them against Invasion; and on Application of the Legislature, or of the Executive (when the Legislature cannot be convened), against domestic Violence.Article. V.The Congress, whenever two thirds of both Houses shall deem it necessary, shall propose Amendments to this Constitution, or, on the Application of the Legislatures of two thirds of the several States, shall call a Convention for proposing Amendments, which, in either Case, shall be valid to all Intents and Purposes, aPart of this Constitution, when ratified by the Legislatures of three fourths of the several States, or by Conventions in three fourths thereof, as the one or the other Mode of Ratification may be proposed by the Congress; Provided that no Amendment which may be made prior to the Year One thousand eight hundred and eight shall in any Manner affect the first and fourth Clauses in the Ninth Section of the first Article; and that no State, without its Consent, shall be deprived of its equal Suffrage in the Senate.Article. VI.All Debts contracted and Engagements entered into, before the Adoption of this Constitution, shall be as valid against the United States under this Constitution, as under the Confederation.This Constitution, and the Laws of the United States which shall be made in Pursuance thereof; and all Treaties made, or which shall be made, under the Authority of the United States, shall be the supreme Law of the Land; and the Judges in every State shall be bound thereby, any Thing in the Constitution or Laws o";
    
    Inode inode;
    size_t inode_number;
    if(load_inode(fs, arg, &inode_number, &inode)){
        check_pass(inode.pass, 1); //calls check pass with password passed
    }

    char writing[BLOCK_SIZE*POINTERS_PER_INODE];
    fgets(writing, BLOCK_SIZE*POINTERS_PER_INODE, stdin);

     ssize_t ret1 = fs_write(fs, arg, 5, writing, strlen(writing), fs_stat(fs, arg));
        if(ret1 > 0)
            printf("\nWrite completed. Wrote %ld bytes.\n", ret1);
       else if(ret1 ==0)
            printf("\nWrite completed. No data was written.\n");
        else 
             printf("\nWrite Failed.\n");
    
    
}

void do_help(){
    printf("Commands are:\n");
    printf("    format\n");
    printf("    mount\n");
    printf("    create <file name>\n");
    printf("    remove  <file name>\n");
    printf("    cat     <file name>\n");
    printf("    write     <file name>\n");
    printf("    help\n");
    printf("    quit\n");
}
