#!/usr/local/bin/perl

# This script runs a few simple tests on a set of *.f and *.f90 files:
#   mfef90 <file>.f<90>
#   whirl2f <file>.B
#   f90 -o <file>.exe <file>.w2f.f<90>
#   <file>.exe
# All test output is saved in multiple files in the log directory. The diagnostic 
# output of this script is saved in the file runtests.log in the current directory.
#
# Usage:
#   runtests.pl [-option_name=option_value] 
# The following command line options are recognized:
#   -f90dir=path  : top-level directory for the installation of the f90 front-end;
#                   by default, the location of mfef90 is <f90dir>/crayf90/sgi/mfef90
#                   and the location of whirl2f is <f90dir>/whirl2f/whirl2f
#   -mfef90=path  : mfef90 executable
#   -whirl2f=path : whirl2f executable
#   -f90=path     : f90 compiler
#   -logdir=path  : directory for storing test logs and reports
#   -srcdir=paths : comma-separated list of directories containing Fortran 90 source code
#   -tests=list   : comma-separated list of tests to run (no spaces); valid options 
#                   are: mfef90, whirl2f, compile, run
#   -clean        : clean up all generated files before script exits (logs are not deleted)
#

# Handle command line options
# (if desired, edit default values below to specify correct paths)

$clean = 0; @sourcedirs = ();
for ($i = 0; $i <= $#ARGV; $i++) {
  if ($ARGV[$i] =~ /-f90dir=(\S+)/) {
    $f90dir = $1; next;
  }
  if ($ARGV[$i] =~ /-mfef90=(\S+)/) {
    $mfef90 = $1; next;
  }
  if ($ARGV[$i] =~ /-whirl2f=(\S+)/) {
    $whirl2f = $1; next;
  }
  if ($ARGV[$i] =~ /-f90=(\S+)/) {
    $f90 = $1; next;
  }
  if ($ARGV[$i] =~ /-logdir=(\S+)/) {
    $logdir = $1; next;
  }
  if ($ARGV[$i] =~ /-srcdir=(\S+)/) {
    @sourcedirs = split ',', $1; next;
  }
  if ($ARGV[$i] =~ /-tests=(\S+)/) {
    $tests = $1; next;
  }
  if ($ARGV[$i] =~ /-clean/) {
    $clean = 1; next;
  }
  &msg("Unrecognized option: $ARGV[$i]\n");
  &usage();
  exit(1);

}	
$f90dir = "/home/radu/Open64/osprey1.0/targ_sparc_solaris" if ($f90dir eq "");
$mfef90 = "$f90dir/crayf90/sgi/mfef90" if ($mfef90 eq "");
$whirl2f = "$f90dir/whirl2f/whirl2f" if ($whirl2f eq "");
$f90 = "f90 -ansi" if ($f90 eq "");
$logdir = "logs" if ($logdir eq ""); system("mkdir -p $logdir") if ( ! -e $logdir );
if ($#sourcedirs == -1) { $sourcedirs[0] = `pwd` ; chomp $sourcedirs[0];}
$tests = "mfef90,whirl2f,compile,run" if ($tests eq "");
$mainlog = "runtests.log"; open LOG, ">$mainlog"; close(LOG);
# end of configuration and command line option handling

foreach $sourcedir (@sourcedirs) {
  opendir FDIR, $sourcedir or die "Could not open source directory: $!";
  @allfiles = readdir FDIR;
  close FDIR;
  print "doing dir $sourcedir\n";
  foreach $file (@allfiles) {
    exit 0 if ($tests eq "");
    if ($file =~ /^(\w+?)\.(f)$/ || $file =~ /^(\w+?)\.(f90)$/) {
      $base = $1; $suffix = $2;
      &msg("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
      &msg(" Starting tests on $file");
      &msg("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
      
      #$command = "$mfef90 -uall $file";
      #$output = &run_test($command);
      
      $intermediate = "$base".".B";
      unlink $intermediate if ( -e $intermediate);
      if ($tests =~ /mfef90/) {
	$command = "$mfef90 $file";
	$logfile = "$logdir/$base".".mfef90.log";
	$out = &run_test($command, $logfile);
	if ( -e $intermediate && $out eq "") {
	  &msg("\t....... mfef90 test \t\t\tPASSED");
	} elsif ( $out ne "") {
	  &msg("\t....... mfef90 test produced output (see $logfile)");
	} else {
	  &msg("\t....... mfef90 test \t\t\tFAILED (see $logfile)");
	}
      }
      
      $newfile = "$base".".w2f.f";
      unlink $newfile if ( -e $newfile);
      if ($tests =~ /whirl2f/) {
	if ( -e $intermediate) {
	  $command = "$whirl2f $intermediate";
	  $logfile = "$logdir/$base".".whirl2f.log";
	  $out = &run_test($command, $logfile);
	  if ( -e $newfile) {
	    &msg("\t....... whirl2f test \t\t\tPASSED");
	  } else {
	    &msg("\t....... whirl2f test \t\t\tFAILED (see $logfile)");
	  }    
	} else {
	  &msg("\t....... whirl2f test \t\t\tFAILED (no $interemediate found)");
	}
      }
      
      
      # Compile
      $execfile = "$base".".exe";
      unlink $execfile if ( -e $execfile);
      if ($tests =~ /compile/) {
	if ( -e $newfile ) {
	  $command = "$f90 -o $execfile $newfile";
	  $logfile = "$logdir/$base".".f90.log";
	  $out = &run_test($command, $logfile);
	  if ($out ne "") {
	    if ( -e $execfile ) {
	      &msg("\t....... f90 test \t\t\tPASSED (output produced, see $logfile)");
	    } else {
	      &msg("\t....... f90 test \t\t\tFAILED (see $logfile)");
	    }
	  } else {
	    &msg("\t....... f90 test \t\t\tPASSED");
	  }
	} else {
	  &msg("\t....... f90 test \t\t\tFAILED (no $newfile to compile)");
	}
      }
      
      # Run test executable
      if ($tests =~ /run/) {
	if (-e $execfile) {
	  $command = "./$execfile";
	  $logfile = "$logdir/$base".".run.log";
	  $out = &run_test($command, $logfile);
	  # Check output
	  if ($out =~ /OK/) {
	    &msg("\t....... Executable test \t\tPASSED");
	  } else {
	    &msg("\t....... Executable test \t\tFAILED (see $logfile)");
	  }
	} else {
	  &msg("\t....... Executable test \t\tFAILED (no $execfile to execute)");
	}
      }
      
    }
    system("rm -f *.B *.w2f.* *.exe *.mod") if ($clean);
    close(LOG);
  }
}


sub run_test() {
  my $command = $_[0];
  my $logfile = $_[1];

  &msg("command: $command");

  # Save STDOUT and STDERR
  open(OLDOUT, ">&STDOUT");
  open(OLDERR, ">&STDERR");

  # Redirect STDOUT and STDERR to log file
  open(STDOUT, ">$logfile") || die "can't redirect STDOUT: $!";
  open(STDERR, ">&STDOUT") || die "can't dup STDOUT: $!";

  select(STDERR); $| = 1;
  select(OUTPUT); $| = 1;

  print STDOUT "\ncommand: $command\n\n";
  system($command);

  close(STDOUT); close(STDERR);
  
  # Restore STDOUT and STDERR
  open(STDOUT, ">&OLDOUT");
  open(STDERR, ">&OLDERR");

  $output = &get_output($logfile);
  return $output;
}

sub msg() {
  my $message = $_[0];
  print STDOUT "$message\n";
  open LOG, ">>$mainlog";
  print LOG "$message\n";
  close(LOG);
  return 1;
}

sub get_output() {
  my $filename = $_[0];
  my $output = "";
  open INPUT, "<$filename";
  while (<INPUT>) {
    $output .= $_ if (!(/^command:/ || /^\s*$/));
  }
  return $output;
}

sub usage {
  print "Usage: \n",
         "   runtests.pl [-option_name=option_value]\n";
  
  print "Valid options are: \n",
    "  -f90dir=path  : top-level directory for the installation of the f90 front-end;\n",
    "                  by default, the location of mfef90 is <f90dir>/crayf90/sgi/mfef90\n",
    "                  and the location of whirl2f is <f90dir>/whirl2f/whirl2f\n",
    "  -mfef90=path  : mfef90 executable\n",
    "  -whirl2f=path : whirl2f executable\n",
    "  -f90=path     : f90 compiler\n",
    "  -logdir=path  : directory for storing test logs and reports\n",
    "  -srcdir=paths : comma-separated list of directories containing Fortran 90 source code\n",
    "  -tests=list   : comma-separated lists of tests to run (no spaces); valid options\n",
    "                  are: mfef90, whirl2f, compile, run\n",
    "  -clean        : clean up all generated files before script exits (logs are not deleted)\n";
}

