#!/usr/bin/perl
use File::Basename;
#/home/edscott/GIT/xffm+/structure/parse11.pl --include=../../include lswfC.cc --templates=/home/edscott/GIT/dune2.6/dumux/
#/home/edscott/GIT/xffm+/structure/parse11.pl lswf-chem12.cc --include=../../../include --templates=/home/edscott/GIT/dune/dumux/

# 1. Process arguments
#    a. $startFile
#    b. $includePath
#    c. $templatePath
#    d. $problemTypeTag

# 2. Get include files in order
#    a. $sourceDir
#    b. @files

# 3. Get typetags
#    a. from macros
#    b. from structures

# 4. Get typeinherits
#
# 5. Get properties
# 6. Get propertyvalues
# 7. Mark focus
# 8. Print XML



# Global variables:
# 1.
$includePath;
$templatePath;
$problemTypeTag;
# 2.
$sourceDir;
@files;
%pathHash;
%includes;
# 3.
@typeTags;
%includes;
$referenceLineCount;



$count;
$debug=0;
$verbose=0;
@files;
%files;

%typeTagFiles;
%typeTagLineNumber;
%properties;
%propertyValues;
%propertyTypeTag;
%propertySource;
%propertyLineNumber;
%inherits;
%focus;
@namespace;



######################## 0. General purpose subroutines
####################################################################################
# Global variables:
$printWarnings=1;
$printDbg=1;
$printTrace=0;
$printInfo=1;

sub usage {
    print "Usage: $0 <target file> \n     [--templates=<systemwide template include directory>]\n     [--include=<local include directory>]\n     [--problemTypeTag=<DuMuX problem TypeTag>]\n";
    exit 1;
}

sub trace{
    if (not $printTrace){return}
    my ($msg) = @_;
#    print "TRACE> *** $msg \n"
}
   
sub dbg{
    if (not $printDbg){return}
    my ($msg) = @_;
    print "DBG> *** $msg \n"
}

sub info{
    if (not $printInfo){return}
    my ($msg) = @_;
    print "Info> $msg\n"
}

sub warning{
    if (not $printWarnings){return}
    my ($msg) = @_;
    print "Warn> $msg\n"
}

######################## 1. Process arguments
####################################################################################

sub processArguments {
    my $start="";
    my $realpath="";
    foreach $arg (@ARGV){
        if ($arg =~ m/^--include/g){
            ($a,$b)=split /=/,$arg,2;
            if (not -d $b){
                print "$b is not a directory\n";
                usage
            } else {
                $includePath = $b;
                trace "local includes: $includePath\n";
            }
            next
        }
        elsif ($arg =~ m/^--templates/g){
            ($a,$b)=split /=/,$arg,2;
            if (not -d $b){
                print "$b is not a directory\n";
                usage
            } else {
                $templatePath = $b
            }
            next
        }
        elsif ($arg =~ m/^--problemTypeTag/g){
            ($a,$problemTypeTag)=split /=/,$arg,2;
            next
        }
        elsif (-e $arg) {
            $start = $arg;
	    $realpath = `realpath $start`;
	    chop $realpath;
            next
        } 
        print "Invalid option: $arg\n";
        usage
    }
    if ($start eq ""){usage}
    return $realpath;
}

# Find problem TypeTag from current file...
sub getProblemTypeTag {
    undef $problemTypeTag;
    my($inFile) = @_;
    my $line=1;
    my $a, $b, $c;
    open IN, "$inFile" or die "getProblemTypeTag:: Cannot open $inFile for processing.\n";
    while (<IN>){
        s/^\s+//;
        if (/^\/\//){next}
        # 2.12 method
        if (/typedef/ and /TTAG/ and /TypeTag/){
            ($a,$b) = split /\(/, $_, 2;
            ($problemTypeTag,$c) = split /\)/, $b, 2;
            info "2.12: Problem TypeTag = \"$problemTypeTag\" from file $inFile line $line-->\n";
            break;
        }
	# 3.0 method
        if (/using/ and /TypeTag/ and /TTAG/){
            ($a,$b) = split /TTAG\(/, $_, 2;
            ($problemTypeTag,$c) = split /\)/, $b, 2;
            info "3.0: Problem TypeTag = \"$problemTypeTag\" from file $inFile line $line-->\n";
            break;
        }
        $line++;
    }
    close IN;
    if (not $problemTypeTag){
        info "Dumux::Problem TypeTag not found.";
    }
}

# Find Dumux installation path from current file...
sub getInstallationPath {
    my ($infile) = @_;
    my $currentDir = dirname($infile);
    my $realpath = &realpath($currentDir);
    my $test = $realpath . "/dumux";
    if (-d $test) {
        info "Assuming installation templates at \"$test\"";
        return $test
    }
    if ($realpath eq "/") {
        info "Dumux installation templates not found";
        return $realpath
    }
    return getInstallationPath($realpath);
}

sub resolveMissingArguments{
    my($start) = @_;
    if ($problemTypeTag){
        info("ProblemTypeTag manually specified to $problemTypeTag");
    } else {
        warning("Will try to determine ProblemTypeTag from $start");
        getProblemTypeTag($start);
    }
    if ($includePath){
        info("Additional include directory at $includePath");
    } else {
        warning("Additional include directory not specified");
    }
    if ($templatePath){
        info("Installed templates at $templatePath");
    } else {
        warning("Installed template location not specified");
        warning("Will try to determine location from $start");
        $templatePath = getInstallationPath($start);
    }

}

######################## 2. Get include files in order
####################################################################################

sub readFiles {
    my ($path, $parentPath) = @_;
#   path array
    push(@files, $path);
#   array of path hashes (contain included files)
    push(@{ $files{$parentPath} }, $path);
#dbg "adding $path to hash($parentPath)";
    &readFile($path);    
}

sub processNextFile {
    my ($nextFile, $path, $text) = @_;
    my $realNextFile = `realpath -q $nextFile`;
    if ($realNextFile =~ m/\n$/) {chop $realNextFile}
    trace ("realNextFile = \"$realNextFile\"");
    if (-e $realNextFile ){ #it exists...
	if ($includes{$realNextFile}) {return 1}
	$includes{$realNextFile} = $text;
	readFiles($realNextFile,$path);
	return 1;
    }
    return 0;
}

sub readFile {
    my ($path) = @_;
    trace "parsing $path...";
    my $stream;
    open $stream, $path or die "cannot open \"$path\"\n";
    my  $a, $b, $c, $d;
    my $comment = 0;
    my $text;
    while (<$stream>){
        s/^\s+//;
        if (/^\/\//){next}
        if (/\/\*/) {$comment = 1}
        if (/\*\//) {$comment = 0}
        if ($comment){next}
        if (not /^#/) {next}
        if (not /#include/) {next}
        if (/"/ or (/</ and />/)) {
            if (/</ and />/)  {
                $text = "absolute";
                ($a, $c) = split /</, $_, 2;
                trace "a c = $a $c";
                ($b, $a) = split />/, $c, 2;
                trace "b a  = $b $a";
                $b =~ s/^\s+//;
            } else {
                $text = "relative";
#               relative includes...
                ($a, $b, $c) = split /"/, $_, 3;
                trace "relative includes... a b c = $a $b $c";
                $b=~ s/^\s+//;
            }
#           1. if in current or relative directory, use it.
	    if ($text eq  "relative"){          
		my $dirname = dirname($path);	    
		my $nextFile = $dirname . "/" . $b;
		if (processNextFile($nextFile, $path, $text)){next}
	    }
#           2. try include path (user templates)
            if ($includePath) {
                $nextFile = $includePath . "/" . $b;
		my $realNextFile = `realpath -q $nextFile`;
		if (processNextFile($nextFile, $path, $text)){next}
            }
#
#           3. try templates path (installation templates)
            if ($templatePath) {
                $nextFile = $templatePath . "/" . $b;
		my $realNextFile = `realpath -q $nextFile`;
		if (processNextFile($nextFile, $path, $text)){next}
            }

#           4. skip system includes
            $nextFile = "/usr/include/" . $b;  
            if (-e $nextFile) {
                print "<!-- skipping header $nextFile -->\n";
                next
            }  
            if ($b =~ m/^dune/g) {next} # ignore dune 
            if (not $b =~ m/\.h/g) {next} # ignore stdc++
            warning "omitting <$b> referenced in $path";          
        }
    }
    close $stream;
}


sub getIncludeFileArray {
    my($start) = @_;
    readFiles($start, "--");
}


######################## 3. Get typetags
####################################################################################

sub getRawLine {
# Returns logical line.
    my $nextLine;
    my $rawline = <INPUT>;
    $referenceLineCount++;

#remove initial whitespace
    $rawline =~ s/^\s+//;

    trace("getRawLine, line: $referenceLineCount\n");

# zap embedded C comments:
    if ($rawline =~ m/\/\*.*\*\//){
        trace "zap embedded comment: $_";
        $rawline =~ s/\/\*.*\*\///g;
        trace "zap result: $rawline";
    }
#   If we have a C comment initiator, continue until terminator.
    my $startComment = 0;
    if ($rawline =~ /\/\*.*/){
        $startComment = 1;
        $rawline =~ s/\/\*.*//;
        while ($startComment == 1){
            $nextLine = getRawLine;
            if ($nextLine =~ m/.*\*\//){
                $nextLine =~ s/.*\*\///;
                $startComment = 0;
                $rawLine .= $nextLine;
            }
        }
    }

# zap full line C++ comments:
    if ($rawline =~ m/^\/\/.*/){
        trace "zapped comment: $rawline";
        $rawline = "\n";
    }

# zap trailing C++ comments:
    if ($rawline =~ m/\/\/.*$/){
        my ($a, $b) = split /\/\//, $rawline, 2;
        trace "zapped trailing comment: $b";
        $rawline = "$a\n";
    }

# join escaped \n lines:
    if ($rawline =~ m/\\\n$/) {
        $rawline =~ s/\\\n$/ /;
        $nextLine = &getRawLine;
        $rawline .= $nextLine;
        trace "join this with previous line: $nextLine";
        trace "join result: $rawline";

    }
    trace("$rawline");
    return $rawline;
}

# get nonescaped multiline command (; token) 
# FIXME: combine with getRawLine()
sub getFullLine{
    my $a,$b;
    my $line;
    if (/\/\//) {($line, $b) = split /\/\//, $_, 2}
    else {$line = $_}
    $line =~ s/\n/ /g;
    $line =~ s/^\s+//;
loop:
    if (not $line =~ m/;/g){
        my $nextLine = <INPUT>;
        $referenceLineCount++;
        $nextLine =~ s/\n/ /g;
        $nextLine =~ s/^\s+//;
        if ($nextLine =~ m/\/\//g){
            ($a,$b) = split /\/\//, $nextLine, 2;
            $nextLine = $a;
        }
        $line = $line . $nextLine;
        goto loop;
    }
    return $line;
}

sub getTagName {
    my $a, $b, $c;
    my($string) = @_;
    ($a, $b) = split /\(/,$string,2;
    if ($b =~ m/,/g){($c, $a) = split /,/,$b,2}
    else {($c, $a) = split /\)/, $b,2}
    $c =~ s/^\s+//;
    return $c;
}


sub getFileMacroTags {
    my ($file) = @_;
    open INPUT, "$file" or die "Unable to open $file";
    my $typeTag;
    my $found = 0;
    my $lineNumber=0;
    my @fileTypeTags;
    $i=0;
    $referenceLineCount=0;
    while (<INPUT>){
        $referenceLineCount++;
        $_ = getRawLine;
	dbg "rawline: $_";
        if (/^#/){next}
        if (/NEW_TYPE_TAG/){
            my $line = getFullLine; 
            $typeTag = getTagName($line);
#print "typetag=$typeTag\n";
            if ($typeTagFiles{$typeTag}) {
                warning("TypeTag \"$typeTag\"redefined at file $file:$referenceLineCount");
#                exit(1);
            } else {
                $typeTagFiles{$typeTag} = $file; 
                $typeTagLineNumber{$typeTag} = $referenceLineCount; 
            }
#            $fileTypeTags[$i++] = $typeTag;
            push @fileTypeTags, $typeTag;
            $found = 1;
        }
    }
    close INPUT;
    if (not $found) {
        return undef
    }
    return @fileTypeTags;
}


sub getMacroTags {
    my ($file) = @_;
    dbg "getMacroTags $file";
    $count++;
    my $path = "$file";

    my @macroTags = getFileMacroTags($path);
    my $tag;
    if (@macroTags){ 
        foreach $tag (@macroTags){
            if ($tag eq "") {next}
            dbg "Found 2.12 TypeTag: $tag ($file)";
            push @typeTags, $tag
        }
    }
    return;
}



sub fullNamespace {
    my @ns = @_;
    my $fullns="";
    foreach $ns (@ns) {
        if ($fullns eq ""){ 
            $fullns = $ns;
        }
        else {$fullns .= "::$ns"}
        $fullns =~ s/\s+//; #hack
    }
    return $fullns;
}
sub compactTemplate{
    my ($ttype) = @_;
    chop $ttype;
    $ttype =~ s/^\s+//;
    $ttype =~ s/^template//g;
    $ttype =~ s/<//g;
    $ttype =~ s/>//g;
    $ttype =~ s/class//g;
    if ($ttype =~ m/{/g){$ttype =~ s/{//g;}
    $ttype =~ s/\s+//;
    return $ttype;
}

sub compactNamespace{
    my ($namespace) = @_;
    $namespace =~ s/^\s+//;
    $namespace =~ s/^namespace//g;
    chop $namespace;
    if ($namespace =~ m/{/g){$namespace =~ s/{//g;}
    $namespace =~ s/\s+//;
    return $namespace;
}

sub getStructTags{
    my $fullns;
    my $level=-1;
    my $nslevel=-1;
    push(@namespace, "");
    my ($file) = @_;
    trace("getStructTags($file)");
    open INPUT, "$file" or die "Unable to open $file";
    $referenceLineCount=0; # global for multilines
    my $templateParameter = "Type"; #default
    my $template;

    while (eof(INPUT) != 1) {
#       Get next logical line.
        $_ = &getRawLine;
#       Skip compiler directives
        if (/^#/ or /^\/\//){next}
   
        if (/^template/){
#get template line
#get template parameter
        }
        if (not /^struct/){
            if (/^template/){
                chop;
                s/^\s+//;
                s/\s+$//;
                $template = $_;
                next;
            } else {
                $template = "";
            }
            if (/^namespace/){
                my $namespace = &compactNamespace($_);
                push(@namespace, $namespace);
                $nslevel = $level+1;
                $fullns = &fullNamespace(@namespace);
                trace "NS: $fullns\n";
            }    
            if (/{/){
                $level++; 
                $fullns = &fullNamespace(@namespace);
                trace "level+=$level $fullns ($namespace[-1])\n";
            }
            if (/}/){
                $level--; 
                if ($level < $nslevel){
                    pop(@namespace);
                    $nslevel--;
                }
                $fullns = &fullNamespace(@namespace);
                trace "level-=$level $fullns ($namespace[-1])\n";
            }
            next
        }

        my $line = &getFullStruct;
#if template, use template 
        trace "line[$referenceLineCount] = $line\n";
        &parseStruct($file, $line, $template);
    }
    close INPUT;
    return;
#    return @fileStructs;
}

sub parseStruct {
    my ($file, $line, $template) = @_;
    my $fullns = &fullNamespace(@namespace);
    dbg "$fullns ----$template structure at\n$file:$referenceLineCount\n$line\n";
}

sub getStructLevel {
    my ($line) = @_;
    my $plus = () = $line =~ /{/g;
    my $minus = () = $line =~ /}/g;
    return $plus - $minus;
}

sub getFullStruct{
    my $line = $_;
    my $start = 0;
    if (not $line =~ m/{/g){ $start=1}
    while ($start or &getStructLevel($line)){
        $start=0;
        $nextLine = &getRawLine;
        $line =~ s/\n/ /g;
        $line = $line . $nextLine;
    }
    return $line;
}

sub printXML {
    my ($start) = @_;
    # print "printXML($start)\n";
    open OUTPUT, ">./structure.xml" or die "cannot open structure.xml";
    print OUTPUT <<EOF;
<?xml version="1.0"?>
<structure-info xmlns:xffm="http://www.imp.mx/">
<structure source=\"$ARGV[0]\" templates=\"$templatePath\" include=\"$includePath\"/>
EOF
    if ($debug) {print "printFilesXML $start\n"}
    &printFile($start, 0);
    if ($debug) {print "printPropertiesXML\n"}
    &printPropertiesXML;
    if ($debug) {print "printTypeTagsXML\n"}
    &printTypeTagsXML;
    print OUTPUT "</structure-info>\n";
    close OUTPUT;
}

sub printFile {     
    my ($start, $level) = @_;

    my @keys;
    my $file;
    my @array;
    $i;
    for ($i=0; $i<$level; $i++) {print OUTPUT " "}

    $oFile = $start;
    $oFile =~ s/$templatePath\///;
    if ($includePath) {$oFile =~ s/$includePath\///;}

    my $realpath = `realpath $start`;
    chop $realpath;
#        $oFile = "&lt;$oFile&gt;";
    if ($includes{$start} == 1 ){
        $oFile = "&lt;$oFile&gt;";
    } elsif ($includes{$start} ==2 ){
#        $oFile = "&quot;$oFile&quot;";
    }
    if ($sourceDir) {
        $oFile =~ s/$sourceDir//g;
    }
    print OUTPUT "<files name=\"$oFile\" realpath=\"$realpath\">\n";

    my @array = @{ $files{$start} };
#        print "$level: hash($start) --> $file\n";

    foreach $file (@array){
#        print "$level: hash($start) --> $file\n";

        &printFile($file, $level+1);
    }
    for ($i=0; $i<$level; $i++) {print OUTPUT " "} 
    print OUTPUT "</files>\n";
}





$j=0;
# Global arrays and hashes:

#&main;


sub markFocus {
    my ($focus, $focusLevel) = @_;
    if (not $focus{$focus}){$focus{$focus} = $focusLevel}

    if ($debug){print "focus: $focus\n";}
    my @array = @{ $inherits{$focus} };
    my $subfocus;
    foreach $subfocus (@array){
        &markFocus($subfocus, $focusLevel+1);
    }
}



sub realpath{
    my ($inPath) = @_;
    if (not -e $inPath){
        if (-e $templatePath."/$inPath"){ $inPath = $templatePath."/$inPath"}
        elsif ($includePath){
            if (-e $includePath."/$inPath"){ $inPath = $includePath."/$inPath"}
            else {return $inPath}
        }
    } 
    my $realpath = `realpath $inPath`;
    chop $realpath;
    return $realpath;
}

sub printTypeTagsXML{
    my %repeat;
    my $typetag;
#    my @keys = sort @typeTags;
    my @keys = sort {$focus{$a} <=> $focus{$b}} @typeTags;
    foreach $typetag (@keys){
        if ($repeat{$typetag}) {
            print OUTPUT "<!-- Repeated typetag name: $typetag -->\n";
            next;
        }
        $repeat{$typetag} = 1;
        my $inheritString = &getInheritString($typetag);

        my $oFile = $typeTagFiles{$typetag};
        $oFile =~ s/$templatePath\///;
        if ($includePath) {$oFile =~ s/$includePath\///;}
        if ($focus{$typetag}){ $focusItem = " focus=\"$focus{$typetag}\""}
        else {undef $focusItem;}
        my $realpath = &realpath($oFile);
        if ($focusItem){
            my $spaces = "";
            my $i;
            for ($i=0; $i<$focus{$typetag}; $i++){
                $spaces .= "   ";
            }
            print OUTPUT " <typetag name=\"$spaces$typetag\" inherits=\"$inheritString\" source=\"$oFile:$typeTagLineNumber{$typetag}\" realpath=\"$realpath\" $focusItem>\n";
            my @akeys = keys(%propertyTypeTag);
            my @skeys = sort @akeys;
            my $property;
            foreach $property (@skeys){
                #property info here
                if ($propertyTypeTag{$property} eq $typetag) {
                    my $value = $propertyValues{$property};
                    $value =~ s/</&lt;/g;
                    $value =~ s/>/&gt;/g;
                    $oFile = $propertySource{$property};
                    $oFile =~ s/$templatePath\///;
                    if ($includePath){$oFile =~ s/$includePath\///;}
                    $realpath = &realpath($oFile);
                    print OUTPUT "  <property name=\"$property\" value=\"$propertyValues{$property}\" source=\"$oFile:$propertyLineNumber{$property}\" realpath=\"$realpath\"/>\n";
                }
            }
            print OUTPUT " </typetag>\n";
        }
    }

}

sub printPropertiesXML{
    my $property;
    my @keys = keys (%properties);
    my @properties = sort @keys;
    foreach $property (@properties){
        $typetag = $propertyTypeTag{$property};
        $source = $propertySource{$property};
        my $lineNumber = $propertyLineNumber{$property};
        $oFile = $source;
        $oFile =~ s/$templatePath\///;
        if ($includePath){$oFile =~ s/$includePath\///;}
        $value = $propertyValues{$property};
        $value =~ s/</&lt;/g;
        $value =~ s/>/&gt;/g;
        my $realpath = &realpath($oFile);
        if ($value ne "undefined"){
            print OUTPUT " <property name=\"$property\" typetag=\"$typetag\" value=\"$value\" source=\"$oFile:$lineNumber\" realpath=\"$realpath\"/>\n";
        }
#        print " <property name=\"$property\">\n";
#        print "  <typetag>$typetag</typetag>\n";
#        print "  <value>$value</value>\n";
#        print "  <source>$source</source>\n";
#        print " </property>\n";
    }


}
#########   Types  ##########
#########   Inherits  ##########
sub getTypeInherits{
    my($file) = @_;
    my $path = "$file";


    open INPUT, "$path" or die "Unable to open $path";
    my $comment = 0;
    $referenceLineCount=0;
    while (<INPUT>){
        $referenceLineCount++;
        $_ = &getRawLine;
        s/^\s+//;
        if (/^#/ or /^\/\//){next}
        if (/\/\*/) {$comment = 1}
        if (/\*\//) {$comment = 0}
        if ($comment){next}
        if (/NEW_TYPE_TAG/){
            my $line = &getFullLine;
            my $typeTag = getTagName($line);
            if (/INHERITS_FROM/){
                my @d = &getInherits($typeTag, $line);
                foreach $a (@d){
                    if ($debug)
                      {print "get inherits $typeTag --> $a ($file:$referenceLineCount\n"}
                    push(@{ $inherits{"$typeTag"} }, "$a");
                }
           }
            next;
        }
    }
    close INPUT;
    return;
}

sub getInherits {
    my $typeTag;
    my $string;
    ($typeTag, $string) = @_;
    my $a, $b, $c, @d;
    undef @d;
    ($a, $b) = split /INHERITS_FROM/,$string,2;
    ($a, $c) = split /\(/, $b,2;
    ($a, $b) = split /\)/, $c,2;

    if ($a =~ m/,/){ @d = split /,/, $a} else {$d[0] = $a}
    my $i = 0;
#    print "string=$string";
    foreach $a (@d){ 
#        print "$typeTag --> $a\n"; 
        $d[$i] =~ s/^\s+//;
        $i++;
    }

   return @d;
}

sub printInherits{
    if (not $debug) {
        print "printInherits is a debug function\n";
        return;
    }
    print("INHERITS:\n");
    my $key;
    my @keys = keys(%inherits);
    foreach $key (@keys){
        print $key;
        my @array = @{ $inherits{$key} };
        &printFileArray(@array);
        print "\n";
    }

}

# FIXME: not returning any inheritsString...
sub getInheritString{
    my ($typetag) = @_;
    my $key;
    my $inheritString = "";
    my @array = @{ $inherits{$typetag} };
    foreach $key (@array){
        $inheritString .= "$key, ";
    }
    return $inheritString;

}


#########   Properties  ##########
sub getProperties {
    my ($file) = @_;
    $count++;
    my $path = "$file";


    my @properties = &getFileProperties($path);
    foreach $property (@properties){
        push(@{ $properties{"$property"} }, "$file");
        $propertySource{$property}=$file;
        $propertyValues{$property}="undefined";
    }
    return;
}

sub getFileProperties{
    my ($path) = @_;

    open INPUT, "$path" or die "Unable to open $path";
    my @properties;
    my $i=0;
    my $comment = 0;
    $referenceLineCount=0;
    while (<INPUT>){
        $referenceLineCount++;
        $_ = &getRawLine;
        s/^\s+//;
        if (/^#/ or /^\/\//){next}
        if (/\/\*/) {$comment = 1}
        if (/\*\//) {$comment = 0}
        if ($comment){next}
        if (/NEW_PROP_TAG/){
            my $line = &getFullLine;
            $prop = getTagName($line);
            $properties[$i++] = $prop;
            $propertyLineNumber{$prop}=$referenceLineCount;
        }
    }
    close INPUT;
    if ($i == 0) {
       $properties[$i++] = "User Class Property";
    }
    return @properties;
}

sub printProperties {
    if (not $debug) {
        print "printProperties is a debug function\n";
        return;
    }
    print("PROPERTIES:\n");
    my $property;
    my @keys = keys(%properties);
    foreach $property (@keys){
        print "property: $property :\n";
        my @array = @{ $properties{"$property"} };
        &printFileArray(@array);
        print "\n";
    }
}

sub printFileArray {
    if (not $debug) {
        print "printFileArray is a debug function\n";
        return;
    }
    my @a = @_;
    foreach $a (@a){ print " ($a)\n"}
}

###############  Property values  ########
sub getPropertyValues {
    my ($file) = @_;
    my $path = "$file";
    my %overwrite;

    open INPUT, "$path" or die "Unable to open $path";
    $comment = 0;
    $referenceLineCount=0;
    while (<INPUT>){
        $referenceLineCount++;
        $_ = &getRawLine;
        s/^\s+//;
        if (/^#/ or /^\/\//){next}
        if (/\/\*/) {$comment = 1}
        if (/\*\//) {$comment = 0}
        if ($comment){next}
        if (/SET_TYPE_PROP/ or /SET_INT_PROP/ or /SET_BOOL_PROP/ or /SET_SCALAR_PROP/){
            my $line = &getFullLine;
            my $typetag = getTagName($line);
            my $property = &getProperty($line);
            my $propValue = &getValue($line);

            if ($debug and $verbose){
                if (/SET_BOOL_PROP/ and /Gravity/ ) {
                    print "SET_BOOL_PROP: $file --> \n$_";
                    print "property=$property value=$propValue\n";
                }
            
                if ($propertyValues{$property}) {
                    warning("overwritting $property: \nold value=$propertyValues{$property} new value=$propValue\nold typetag=$propertyTypeTag{$property} new typetag=$typetag\nold source=$propertySource{$property} new source=$file");
                }
            }
            $propertyValues{$property} = $propValue;
            $propertyTypeTag{$property} = $typetag;
            $propertySource{$property} = $file;
            $propertyLineNumber{$property} = $referenceLineCount;
            if ($debug) {
                my $prop;
                if ($overwrite{$property}) {$prop = "PROP*"}
                else {$prop = "PROP*"}
                print("PROP $propertyTypeTag{$property}:$property = $propertyValues{$property} ($propertySource{$property}) \n")
            }
            $overwrite{$property} = 1; 
        }
    }
    close INPUT;
    return;
}

sub getProperty {
    my @a;
    my($string) = @_;
    @a = split /,/,$string,3;
    $a[1] =~ s/^\s+//;
    return $a[1];
}

sub getValue {
    my @a, $b, $c;
    my($string) = @_;
    @a = split /,/,$string, 3;
    $a[2] =~ s/^\s+//;
    chop $a[2]; # \n
    chop $a[2]; # ;
    $a[2] =~ s/^\s+//;
    return $a[2];
}


####################################################################################
####################################################################################
sub main {
    $debug = 0;
# 1.
    my $start = processArguments;
    resolveMissingArguments($start);
    dbg "1 ok";
# 2.
    getIncludeFileArray($start);
    foreach $f (@files){dbg "file: $f"}
    dbg "2 ok";
    exit 1;

# 3. 
#   2.12
    foreach $f (@files){getMacroTags($f)}
    foreach $f (@typeTags) {dbg "tag: $f"}
    
    dbg "3 ok";
    exit 1;
#   3.0
####################
#    &getStructTags($ARGV[0]); exit 1;
    foreach $f (@files){&getStructTags($f)}
    exit(1);
####################

    foreach $f (@files){&getTypeInherits($f)}
    if ($debug){&printInherits}
    $count = 0;
    foreach $f (@files){&getProperties($f)}
    if ($debug){&printProperties}
    if ($debug){print("getPropertyValues:\n")}
    foreach $f (@files){&getPropertyValues($f)}
    if (defined $problemTypeTag){
        &markFocus($problemTypeTag, 1);
    }    
    &printXML($start);
}
####################################################################################
####################################################################################


if (not $ARGV[0]){
    print "Please specify target file.\n";
    usage
}
main;
exit 1;

