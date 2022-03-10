#!/usr/bin/perl

use strict;
use CGI;

my $list = new CGI;

my $fileDir = $ARGV[0];
my @files;

opendir DIR, "$fileDir" or die "Can't open $fileDir $!";
    @files = readdir DIR; # grep { /\.(?:txt|err|csv)$/i } readdir DIR;
closedir DIR;

print # $list->header("text/html"),
      $list->start_html("Archives de $fileDir"),
      $list->p("Voici les fichiers de $fileDir");

# $fileDir =~ s/files/downloads/; # PROBLEM : CANTONNE LA NAVIGATION A DOWNLOADS
foreach my $file (@files) {
    print $list->p(
            $list->a({-href=>'/' . $fileDir . '/' . $file},
            $file)
            );
    
}

print $list->end_html;

#ANCIENNE SOLUTION (en .c):

# DIR *dpdf;
# struct dirent *epdf;
# 
# dpdf = opendir("./");
# if (dpdf != NULL)
#    while ((epdf = readdir(dpdf)))
#      std::cout << epdf->d_name << std::endl;
# closedir(dpdf);

#   use File::Spec;
#   ...
#   my $rel_path = 'myfile.txt';
#   my $abs_path = File::Spec->rel2abs( $rel_path ) ;


#Code Cpp qui va avec ce script 
	# int fd[2], pid;
	# if (pipe(fd) == -1 || (pid = fork()) == -1)
	# 	throw runtime_error("pipe || fork failed");
	# if (pid == 0) {// Child
	# 	char const *argv[] = {"files/cgi/perlFolderLister.pl", _path.c_str(), NULL};
	# 	close(fd[0]);	/* Close unused read end */
	# 	dup2(fd[1], STDOUT_FILENO);
	# 	if (execv(*argv, (char *const *)argv) == -1)
	# 		_exit(EXIT_FAILURE);
	# 	close(fd[1]);
	# }
	# else {
	# 	close(fd[1]);          /* Close unused write end */
	# 	char buf[1000];
	# 	int n;
	# 	_body.clear();
	# 	while ((n = read(fd[0], buf, 999))) {
	# 		buf[n] = '\0';
	# 		_body.append(buf);
	# 	}
	# 	gen_CType("html");
	# 	// gen_CType("html");
	# 	cout <<  CYAN "folder_ response" RESET << _body  <<  CYAN "path_ response" RESET << _path << endl;
	# 	close(fd[0]);          /* Reader will see EOF */
	# 	wait(NULL);            /* Wait for child */
	# 	return ;
	# }