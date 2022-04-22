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

$fileDir =~ s/files/downloads/; # PROBLEM : CANTONNE LA NAVIGATION A DOWNLOADS
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