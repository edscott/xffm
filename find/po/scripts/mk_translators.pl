#!/usr/bin/perl
use File::Basename;
# 
#
sub ltrim($);
$_=`ls *.po`;
    @files=split;
    foreach $file (@files){
	print "making translators translation for $file...\n";
	if (not $file =~ m/\.po$/g) {next}
	$buffer="";
	open INPUT, "$file" or die "cannot open ./$file for input\n";
	undef $already_done;
	undef @translators;
	undef $language;
	$index=0;
	while (<INPUT>){
	    if (/rfm-translation-team/){
		print "rfm-translation-team is already present\n";
		$already_done=1;
		break;
	    }
	    $buffer .= $_;
	    if (/Language-Team:/){
		($dump, $language) = split /Team:/, $_, 2;
	    }
	    if (/^#/ and /\@/){
		if (/EMAIL\@ADDRESS/){next}
		if (/\@TITLE\@/){next}
		if (/Author\@Betawiki/){next}
		if (/Croatian message file for YaST2/){next}
		if (/mailto:/){next}
		if (/#-#-/){next}
		if (/This file is distributed under the same license as \@PACKAGE@ package. FIRST/){next}
		if (/Edscott Wilson/ and $file ne "es.po"){next}
		chop;
		($dump, $translator) =  split / /,$_, 2;
		$_= $translator;
		if (/ranslator:/) {
		    ($dump, $translator) = split /ranslator:/,$_, 2;
		    $_= $translator;
		} 
		$translator =~ s/Copyright//g;
		$translator =~ s/\(C\)//g;
		$translator =~ s/\(c\)//g;
		$translator =~ s/Maintainer://g;
		$translator =~ s/"//g;
		$translator =~ s/'//g;
		$translator =~ s/\\//g;
		$translator =~ s/#//g;
		$translator =~ s/This file is distributed under the same license as the PACKAGE package.//g;
		$translator =~ s/Prevous maintainers://g;
		$translator =~ s/Prevous maintainer://g;
		$translator =~ s/Translated on//g;
		$translator =~ s/Reviewed on//g;
		$translator =~ s/urdated by://g;
		$translator =~ s/4 corrections://g;
		$translator =~ s/Previous translations://g;
		$translator =~ s/translated by//g;
		$translator =~ s/This file is distributed under the same license as the gimp-script-fu package//g;
		$translator =~ s/Translators://g;
		$translator =~ s/FIRST AUTHOR//g;
		$translator =~ s/Copyrigyt//g;
		$translator =~ s/Copywrite//g;
		$translator =~ s/Minor fixes by//g;
		if ( $translator ne "") {
		    $translators[$index++] = $translator;
		}

	    }
	}
	close INPUT;
	if ($already_done){next}
	@lines=split /\n/, $buffer;
	open OUTPUT, ">$file" or die "cannot open $file for output\n";
	foreach $_ (@lines){
	    if ($lastline eq "" and $_ eq "") {next}
	    print OUTPUT "$_\n";
	    $lastline=$_;
	}
	if (@translators){
	    print OUTPUT "\nmsgid \"rodent-translation-team\"\nmsgstr \"\"\n";
	    print OUTPUT "\"*** $language\"\\n\"\n";
	    foreach $_ (@translators){
		$vato = ltrim($_);
		print OUTPUT "\"-  $vato\\n\"\n";
	    }
	    print OUTPUT "\"-------------------------------------\"\n";
	    print OUTPUT "\n";

	}
	close OUTPUT;
    }


sub ltrim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	return $string;
}
