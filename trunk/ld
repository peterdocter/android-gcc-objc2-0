#!/usr/bin/perl
use File::Basename;

@ldargs = ();
($progname, $progpath, $progsuffix) = fileparse($0, ());
push(@ldargs, $progpath . "linker");
@suffixlist = ("LinkFileList");
@args = @ARGV;
foreach $argnum (1 .. $#ARGV)
{
	$argument = $ARGV[$argnum];
	($name,$path,$suffix) = fileparse($argument,@suffixlist);
	if($suffix eq "LinkFileList")
	{
		$contents = `cat "$argument"`;
		@files = split("\n", $contents);
		for $filenum (0 .. $#files)
		{
			$file = $files[$filenum];
			push(@ldargs, $file);
		}
	}
	else
	{
		push(@ldargs, $argument);
	}
}
system(@ldargs) == 0
	or die "$?";
