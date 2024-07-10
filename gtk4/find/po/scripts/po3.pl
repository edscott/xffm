#!/usr/bin/perl
use File::Basename;
#
# Translations with \r will be marked fuzzy.
#
#undef ($enable_rodent);
#
#arg[0], source one directory
#arg[1], source two directory
#arg[2], target output directory.
#

if (not $ARGV[0] or not $ARGV[1] or not $ARGV[2]){
    print "insufficient arguments... (dir1 dir2 targetdir )\n";
    exit 1;
}

mkdir  $ARGV[2];
@dirs = ($ARGV[0],$ARGV[1]);

foreach $dir (@dirs) {
    print "---------  $dir/\n"; #next;
   $_=`ls $dir/*po`;#next;
    @files=split;
    foreach $file (@files){
	if (not $file =~ m/\.po$/g) {next}
	print "processing: $file\n"; #next;
	undef $empty_msgid;
	if (not &is_utf($file)){
	    print "$file is not utf: converting now\n";
	    @n=split /\/\//, $file;
	    open OUTPUT, ">./$input" or die "cannot open ./$input for output\n";
	    print OUTPUT $buffer;
	    close OUTPUT;
	}
	@lines=split /\n/, $buffer;
	$newfile=$ARGV[2]."/".basename($file);
#print "newfile=$newfile\n";
	if ($processed{"$newfile"}) {
	    print "appending to $newfile\n";
	    open NEWFILE, ">>$newfile" or die "cannot open $newfile\n";
	} else {
	    print "creating $newfile ($dir)\n";
	    open NEWFILE, ">$newfile" or die "cannot open $newfile\n";
	    $processed{"$newfile"}=1;
	}
#	if($newfile eq "./mi.po"){$debug=1}
	&parser(@lines);
	undef $debug;
	print NEWFILE "\n";
	close NEWFILE;

#print "$file\n";
    }
}

&write_authors;

exit 0;

#    @files
sub parser{
undef $fuzzy;
$first_fuzzy=1;
$initial_comments=1;
    foreach $_ (@_){
#initial comments:
	if (/^#/){
	    if ($initial_comments and not /fuzzy/) {
#		print NEWFILE "$_\n"; 
	    
	    if (/\@/){
	        ($name,$b)=split /</,$_,2;
		if (not $name){next}
		($email,$date)=split />/,$b,2;
		while ($name =~ m/^ /g){$name =~ s/^ //g}
		while ($name =~ m/ $/g){$name =~ s/ $//g}
		if ($name eq ""){next}
		if (not $emails{"$name"}){$emails{"$name"}="$email";}
		if (not $dates{"$name"}){$dates{"$name"}="$date";}
		else {$dates{"$name"}.="$date";}
		if (not $names_in_file{"$newfile"}{"$name"}){
		  $names_in_file{"$newfile"}{"$name"}=1;
		}
#		$authorhash{"$newfile"}{"$name"}=$emailhash{"$newfile"}{"$name"};
	        #print "credits: $_\n";
	      }
	      next;
	    }
	}
#skip lines:
	if (/^#\|/){next}
	if (/^#~\|/){next}
	if (/^#:/) {next}
	if (/^#\./) {next}
	if (/msgctxt/){$msgctxt=1;next} # don't know what to do with this, at this moment
#if (/msgid_plural/){next}
#fuzzy control:
#	if (/^#, kde-format/){$fuzzy=1; next}
	if (/^# \|,no-bad-patterns/){$fuzzy=1; next}
	if (/fuzzy/) {
	    if (not $first_fuzzy) {$fuzzy=1}
	    next;
	}
	#newline marks end of block
	if ($fuzzy==1 and $_ eq ""){undef $initial_comments;$fuzzy=0;$msgid=0;$msgstr=0;next}
	    if ($debug){print "2->$newfile: $_\n"}
	if ($fuzzy==1){next}
	    if ($debug){print "3->$newfile: $_\n"}
	#substitutions:
	if (/^#~ /){ s/^#~ //g;	}

	# skip all other comments
	if (/^#/) {next}
	if ($debug){print "4->$newfile: $_\n"}
	
	if (/\r/){s/\r//g;}

	if (/^"Project-Id-Version:/){
	    $_="\"Project-Id-Version: Rodent Delta\\n\"";
	}
	if (/^"Report-Msgid-Bugs-To:/){
	    $_="\"Report-Msgid-Bugs-To: http://bugs.xffm.org\\n\"";
	}
	#newline marks end of block:
	if ($_ eq "" and $msgstr) { 
	    undef $initial_comments;
	    $fuzzy=0;$msgid=0;$msgstr=0;
	    #ignore non-translated strings
	    if ($msgstr eq "msgstr \"\"") {next}
	    #check if in hash
	    if ($id =~ m/msgid_plural/g) {
		$is_plural=1;
		($key,$dump)=split /msgid_plural/,$id,2;
	    } else {
		undef $is_plural;
		$key=$id
	    }
# eliminate trailing whitespace from key
	    $key =~ s/\n//g;
	    $key =~ s/\s+$//; 
	    $key =~ s/"//g;

	    $translation=$msg;
	    $translation =~ s/"//g;
	    $translation =~ s/\n//g;
	    $translation =~ s/ //g;
	    if ($translation eq "msgstr") {
#		print "No translation for $id";
		next
	    }
#	    do kde format changes:
	    if ($is_plural) {
		$key =~ s/%1/%d/g;
		$id =~ s/%1/%d/g;
		$msg =~ s/%1/%d/g;
	    } else {
		$key =~ s/%1/%s/g;
		$id =~ s/%1/%s/g;
		$msg =~ s/%1/%s/g;
	    }
	    if ($is_plural) {
		$key =~ s/%2/%d/g;
		$id =~ s/%2/%d/g;
		$msg =~ s/%2/%d/g;
	    } else {
		$key =~ s/%2/%s/g;
		$id =~ s/%2/%s/g;
		$msg =~ s/%2/%s/g;
	    }
	    if ($is_plural) {
		$key =~ s/%3/%d/g;
		$id =~ s/%3/%d/g;
		$msg =~ s/%3/%d/g;
	    } else {
		$key =~ s/%3/%s/g;
		$id =~ s/%3/%s/g;
		$msg =~ s/%3/%s/g;
	    }
	    
# other kinky KDE format changes:
	    $key =~ s/{[0-9]}/%s/g;
	    $id =~ s/{[0-9]}/%s/g;
	    $msg =~ s/{[0-9]}/%s/g;
	    

# remove specific markups
	    @markups=("&","<b>","</b>","<br />","<numid>","</numid>",
		    "<br/>","</br>","<qt>","</qt>","<br>",
		    "<filename>","</filename>","<ul>", "</ul>",
		    "<PRIuMAX>","<small>","</small>", "<i>", "</i>",
		    "<p>","</p>","<strong>", "</strong>", "<h2>", "</h2>",
		    "<B>","</B>","<big>","</big>");
	    foreach $_ (@markups){
		$key =~ s/$_//g;
		$id  =~ s/$_//g;
		$msg =~ s/$_//g;
	    }
# hacky substitution
	    if (not $is_plural){
		$key =~ s/_//g;
		$id  =~ s/_//g;
		$msg =~ s/_//g;
	    }
#	    // if not in hash, print to file
	    if (not $idhash{"$newfile"}{"$key"}) {
		    $idhash{"$newfile"}{"$key"}=1;
		    #first string is fuzzy, regardless
		    if ($first_fuzzy and $id eq "msgid \"\"\n"){
			print NEWFILE "#, fuzzy\n";
			print NEWFILE "$id";
			print NEWFILE "$msg\n";
		    } 
		    else {
		      undef $is_wrong;
		      $_ = $msg;
		      if (/msgstr\[0\]/){
			  $_ = $id;
			  if (not /msgid_plural/){
			    $is_wrong = 1;
			    $id =~ s/\n/ DUMP /g;
			    print "DUMP: id=$id\nDUMP: msg=$msg\n";
			  }
		      } 
		      $_ = $msg;
		      if (/msgidplural/){
			    print "DUMP: id=$id\nDUMP: msg=$msg\n";
			  $is_wrong = 1;
		      }

		      if ($id ne "msgid \"\"\n" and not $is_wrong){
# Warnings management:
#		    	$msg =~ s/\r//g;
#		    	$msg =~ s/\a//g;
#		    	$msg =~ s/\v//g;
#		    	$msg =~ s/\b//g;

# Message acceptance:
			print NEWFILE "$id";
			print NEWFILE "$msg\n";
		      }
		    }
	    } 
	    elsif ($debug){print "$newfile: key already in hash!\n"}
	    undef $first_fuzzy;
	    next;
	} 
	#msgid and msgstr:
	if (/^msgid/ and not /^msgid_plural/){
		$msgid=1;$msgstr=0;$id="";$msg="";
		undef $msgctxt;
	}
	if (/^msgstr/){$msgstr=1;$msgid=0;}
	if ($msgid==1){
	    $id .= "$_\n";
	    next;
	}
	if ($msgstr==1){
	    $msg .= "$_\n";
	    next;
	}
	#print anything else:
	if (not $msgctxt and not /^msgid_plural/) {
#		print NEWFILE "$_\n";
#	    print "$_\n";
	}
    }
}


sub convert_utf8{
    my ($input,$charset)=@_;
    $buffer=`iconv -f $charset -t utf-8 $input`;
    $buffer =~ s/charset=$charset/charset=UTF-8/g;
}


sub is_utf{
    my $charset;
    my ($input)=@_;
	
# Hack: consider all files utf-8
    $buffer=`cat $input`;return TRUE;
    
    open INPUT, $input or die "cannot open $input\n";
    while (<INPUT>) {
	if (/Content-Type/){
	    close INPUT;
	    if (/charset=utf-8/ or /charset=UTF-8/){
		$buffer=`cat $input`;
		return TRUE;
	    } else {
		($dump,$b)=split /charset=/,$_,2;
		($charset, $dump)=split /\\n/,$b,2;
#print "$charset: $_";
		print "converting $charset: $input\n";
		&convert_utf8($input, $charset);
		return FALSE;
	    }
	}
    }
    print "*** Error determining explicit charset for $input\n";
    close INPUT;
    return TRUE;
}


sub write_authors {
    print "writing author information...\n";
    foreach $key (keys %processed) {
        $input="";
	open INPUT, "$key" or die "cannot open $key for read\n";
	while (<INPUT>) {$input .= $_}
	close INPUT;
	open OUTPUT, ">$key" or die "cannot open $key for write\n";
	$date=`date`; chop $date;
	print OUTPUT<<HEADER;
# Translations for Rodent package.
# Copyright (C) 2004-2012 FSF.
# This file is distributed under the same license as the Rodent package.
#   --- $date ---
#
HEADER
	foreach $name (keys %emails){
		if($names_in_file{$key}{"$name"}){
			$email=$emails{"$name"};
			undef $date;
			undef $c;
			$min=9999;
			$max=0;
			for ($i=1995; $i<=2021; $i++){
			   if ($dates{"$name"} =~ m/$i/g){
			      if ($i < $min) {$min=$i}
			      if ($i > $max) {$max=$i}
			      $c=1;
			   }
			}
			if ($c) {
			   if ($min==$max) {$date = ", $min"}
			   else {$date=", $min-$max"}
			}
			print OUTPUT "$name <$email>$date.\n";
#			print OUTPUT "# $name <$email>, 2011.\n";
		}
	}
	print OUTPUT $input;
	close OUTPUT;
        print " finished $key.\n";
    }

}


