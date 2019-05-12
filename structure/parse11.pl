#!/usr/bin/perl
use File::Basename;
use IO::Handle;

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

#
# 4. Get properties and values
#    a. from macros
#    b. from structures

# 5. Mark focus
# 6. Print XML

######################## 0. General purpose 
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
#
#  Called with processArguments() 
#  Creates:
#    $includePath (if provided with --include=)
#    $templatePath  (if provided with --templates= or maybe found)
#    $problemTypeTag; (if provided with --problemTypeTag= or maybe found)
#
#    
#  Returns:
#    Absolute path of argument provided.
####################################################################################
$includePath;
$templatePath;
$problemTypeTag;

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

######################## 2. Get include files in order at @files
#
# Called with getIncludeFileArray($file) ($file in absolute path).
# Creates:
#   @files: Array or ordered include chain (absolute paths).
#   %files: Hash of arrays, hashed by $file (absolute path).
#   %includes: Hash of include lines, hashed by $file (absolute path).
# Uses:
#   $referenceLineCount
#
###################################################################################
# Recursive read of include chain:
#   getIncludeFileArray
#     readFiles  <----------|
#       readFile            |
#	  processNextFile   |
#           readFiles ------|
#   createFilesArray
@files;
%files;
$referenceLineCount;
%includes;

sub readFiles {
    my ($path, $parentPath) = @_;
#   array of path hashes (each will contain the included files for $parentPath)
    push(@{ $files{$parentPath} }, $path);
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
#push(@files, $path);
	readFiles($realNextFile,$path);
	return 1;
    }
    return 0;
}

# Recursive read 
sub readFile {
    my ($path) = @_;
    trace "parsing $path...";
    my $stream;
    open $stream, $path or die "cannot open \"$path\"\n";
    my  $a, $b, $c, $d;
    my $comment = 0;
    my $text;
    my $textType;
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
                ($a, $c) = split /</, $_, 2;
                trace "a c = $a $c";
                ($b, $a) = split />/, $c, 2;
                trace "b a  = $b $a";
                $b =~ s/^\s+//;
                $textType = "absolute";
                $text = "<$b>";
            } else {
#               relative includes...
                ($a, $b, $c) = split /"/, $_, 3;
                trace "relative includes... a b c = $a $b $c";
                $b=~ s/^\s+//;
                $textType = "relative";
                $text = "\"$b\"";
            }
#           1. if in current or relative directory, use it.
	    if ($textType eq  "relative"){          
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

# Create ordered file array from array of hashes.
sub createFilesArray {     
    my ($start, $level) = @_;
   
    my @array = @{ $files{$start} };
    my $file;
    my $src = basename($start);
    if ($level == 0){push(@files, $start)} # top item
    foreach $file (@array){
	push(@files, $file);
        createFilesArray($file, $level+1);
    }
    return;
}

sub printFileOut {     
    my ($out, $start, $level) = @_;
   
    my @array = @{ $files{$start} };
    my $file;
    my $i;
    my $src = basename($start);
    if ($level == 0){# top item
	print $out  "$src\n";
    } 
    foreach $file (@array){
	my $tgt = $includes{$file};
	for ($i=0; $i<$level+1; $i++) {print $out  " "} 
	$tgt =~ s/"/&quot;/g;
	$tgt =~ s/</&lt;/g;
	$tgt =~ s/>/&gt;/g;
	print $out "$tgt\n";
        printFileOut($out, $file, $level+1);
	for ($i=0; $i<$level+1; $i++) {print $out  " "} 
        print $out "</files>\n";

    }
    if ($level == 0){# top item
	print $out  "</files>\n";
    } 
    return;
}

sub getIncludeFileArray {
    my($start) = @_;
    my $level=0;
    readFiles($start, "--");
    printFileOut(STDOUT, $start, $level);
    createFilesArray($start);
#    foreach $f (@files){dbg "file: $f"}
}
######################## 3. Get typetags
#
# Called with getTypeTags().
# Creates:
#   @typeTags
#   @namespace
#   %typeTagFiles
#   %typeTagLineNumber
#   %inherits: Hash of arrays, hashed by $typeTags (member of @typeTags).
#
# Uses:
#   $referenceLineCount
#   @files
# 
####################################################################################
@typeTags;
@namespace;
%typeTagFiles;
%typeTagLineNumber;


# Append lines until token ; reached. 
sub getFullLine{
    my $a,$b;
    my $line = $_;
    $line =~ s/\n/ /g;
    while (not $line =~ m/;/g){
        my $nextLine = &getRawLine;
        $nextLine =~ s/\n/ /g;
        $line = $line . $nextLine;
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

sub compactNamespace{
    my ($namespace) = @_;
    $namespace =~ s/^\s+//;
    $namespace =~ s/^namespace//g;
    chop $namespace;
    if ($namespace =~ m/{/g){$namespace =~ s/{//g;}
    $namespace =~ s/\s+//;
    return $namespace;
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

sub parseStruct {
    my ($file, $line, $template) = @_;
    my $fullns = fullNamespace(@namespace);
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
    while ($start or getStructLevel($line)){
        $start=0;
        $nextLine = &getRawLine;
        $line =~ s/\n/ /g;
        $line = $line . $nextLine;
    }
    return $line;
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
                my $namespace = compactNamespace($_);
                push(@namespace, $namespace);
                $nslevel = $level+1;
                $fullns = fullNamespace(@namespace);
                trace "NS: $fullns\n";
            }    
            if (/{/){
                $level++; 
                $fullns = fullNamespace(@namespace);
                trace "level+=$level $fullns ($namespace[-1])\n";
            }
            if (/}/){
                $level--; 
                if ($level < $nslevel){
                    pop(@namespace);
                    $nslevel--;
                }
                $fullns = fullNamespace(@namespace);
                trace "level-=$level $fullns ($namespace[-1])\n";
            }
            next
        }

        my $line = getFullStruct;
#if template, use template 
        trace "line[$referenceLineCount] = $line\n";
        parseStruct($file, $line, $template);
    }
    close INPUT;
    return;
#    return @fileStructs;
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

sub getMacroTags {
    my ($file) = @_;
    trace "getMacroTags $file";
    open INPUT, "$file" or die "Unable to open $file";
    my $typeTag;
    my $found = 0;
    my $lineNumber=0;
    my @fileTypeTags;
    $referenceLineCount=0;
    while (eof(INPUT) != 1){
        $_ = &getRawLine;
        if (/^#/){next}
        if (not /NEW_TYPE_TAG/){next}
	my $line = getFullLine; 
	dbg "getMacroTags(): full line: $line";
	$typeTag = getTagName($line);
	dbg "getMacroTags(): typetag=$typeTag";
	if ($typeTagFiles{$typeTag}) {
	    warning("TypeTag \"$typeTag\"redefined at file $file:$referenceLineCount");
	} else {
	    $typeTagFiles{$typeTag} = $file; 
	    $typeTagLineNumber{$typeTag} = $referenceLineCount; 
	}
        if (/INHERITS_FROM/){
	    my @d = getInherits($typeTag, $line);
	    foreach $a (@d){
		dbg "  inherits --> \"$a\" ($file:$referenceLineCount";
		push(@{ $inherits{"$typeTag"} }, "$a");
	    }
        }
	if ($typeTag ne "") {push @typeTags, $typeTag}
	
#            $fileTypeTags[$i++] = $typeTag;
#if ($tag ne "") {push @typeTags, $typeTag}
    }
    close INPUT;
    return;
}




sub getTypeTags {
#   2.12 Macro tags
    foreach $f (@files){getMacroTags($f)}
    foreach $f (@typeTags) {dbg "tag: $f"}
    
#   3.0 Structure tags
####################
#    &getStructTags($ARGV[0]); exit 1;
#    foreach $f (@files){&getStructTags($f)}
}

######################## 4.  Get properties
#
# Called with 
# Creates:
#   %propertySource;
#   %propertyValues;
#   %propertyLineNumber;
#   %propertyTypeTag;   
#   %properties: Hash of arrays, hashed by $property (arrays of file paths).
# 
#  
####################################################################################
%propertySource;
%propertyValues;
%propertyLineNumber;
%propertyTypeTag;
%properties;

# Returns an array filled with the properties defined in the file.
sub getProperties{
    my ($path) = @_;

    open INPUT, "$path" or die "Unable to open $path";
    my @properties;
    my $i=0;
    $referenceLineCount=0;
    my $prop;
    while (eof(INPUT) != 1){
        $_ = &getRawLine;
        s/^\s+//;
        if (/^#/){next}
        if (/NEW_PROP_TAG/){
            my $line = &getFullLine;
	    trace "$path:$referenceLineCount $line";
            $prop = getTagName($line);
            $properties[$i++] = $prop;
            $propertyLineNumber{$prop}=$referenceLineCount;
	    $propertySource{$prop}=$path;
	    $propertyValues{$prop}="undefined";
        }
    }
    close INPUT;
    if ($i > 0){
	foreach $prop (@properties){
	    push(@{ $properties{$prop} }, $path);
	    dbg "  $prop: $propertySource{$prop}:$propertyLineNumber{$prop} \"$propertyValues{$prop}\"";
	}
    }
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
    ($a, $b) = split /\(/,$string, 2;
    ($a, $c) = split /\)/,$b, 2;
    @a = split /,/,$a, 3;
    $a[2] =~ s/^\s+//;
    return $a[2];
}

sub printProperties {
    dbg("PROPERTIES:\n");
    my $property;
    my @keys = keys(%properties);
    foreach $property (@keys){
        print "property: $property :\n";
        my @array = @{ $properties{"$property"} };
	my $a;
	foreach $a (@array){ print " ($a)\n"}
        print "\n";
    }
}

sub getPropertyValues {
    my ($file) = @_;
    my $path = "$file";

    open INPUT, "$path" or die "Unable to open $path";
    $referenceLineCount=0;
    while (eof(INPUT) != 1){
        $_ = &getRawLine;
        if (/^#/){next}
        if (/SET_TYPE_PROP/ or /SET_INT_PROP/
		or /SET_BOOL_PROP/ or /SET_SCALAR_PROP/)
	{
            my $line = getFullLine;
            my $typetag = getTagName($line);
            my $property = getProperty($line);
            my $propValue = getValue($line);
            
	    if ($propertyValues{$property}) {
		warning("overwritting $property: \nold value=$propertyValues{$property} new value=$propValue\nold typetag=$propertyTypeTag{$property} new typetag=$typetag\nold source=$propertySource{$property} new source=$file");
	    }
            
            $propertyValues{$property} = $propValue;
            $propertyTypeTag{$property} = $typetag;
            $propertySource{$property} = $file;
            $propertyLineNumber{$property} = $referenceLineCount;
	    dbg("PROP $propertyTypeTag{$property}:$property = $propertyValues{$property} ($propertySource{$property}) \n")
        }
    }
    close INPUT;
    return;
}


######################## 5. Mark focus 
####################################################################################
####################################################################################
%focus;

sub markFocus {
    my ($focus, $focusLevel) = @_;
    if (not $focus{$focus}){$focus{$focus} = $focusLevel}

    dbg "focus: $focus\n";
    my @array = @{ $inherits{$focus} };
    my $subfocus;
    foreach $subfocus (@array){
        &markFocus($subfocus, $focusLevel+1);
    }
}
####################################################################################

######################## 6. Print XML 
####################################################################################
####################################################################################


sub printPropertiesXML{
    dbg "printPropertiesXML";
    my ($out) = @_;
    my $property;
    my @keys = keys (%properties);
    my @properties = sort @keys;
    foreach $property (@properties){
        $typetag = $propertyTypeTag{$property};
        $source = $propertySource{$property};
        my $lineNumber = $propertyLineNumber{$property};
        $oFile = basename($source);
#        $oFile = $source;
#        $oFile =~ s/$templatePath\///;
#        if ($includePath){$oFile =~ s/$includePath\///;}
        $value = $propertyValues{$property};
        $value =~ s/</&lt;/g;
	$value =~ s/>/&gt;/g;
#        my $realpath = &realpath($oFile);
	dbg "property: $property-->$value";
        if ($value ne "undefined"){
            print $out " <property name=\"$property\" typetag=\"$typetag\" value=\"$value\" source=\"$oFile:$lineNumber\" realpath=\"$source\"/>\n";
        }
#        print " <property name=\"$property\">\n";
#        print "  <typetag>$typetag</typetag>\n";
#        print "  <value>$value</value>\n";
#        print "  <source>$source</source>\n";
#        print " </property>\n";
    }


}

sub getInheritString{
    my ($typetag) = @_;
    my $key;
    my $inheritString = "";
    my @array = @{ $inherits{$typetag} };
    my $first = 1;
    foreach $key (@array){
	if ($first) {$inheritString .= "$key"}
	else {$inheritString .= ",$key"}
	undef($first);
    }
    return $inheritString;

}

sub printTypeTagsXML{
    dbg "printTypeTagsXML";
    my ($out) = @_;
    my %repeat;
    my $typetag;
#    my @keys = sort @typeTags;
    my @keys = sort {$focus{$a} <=> $focus{$b}} @typeTags;
    foreach $typetag (@keys){
        if ($repeat{$typetag}) {
            print $out "<!-- Repeated typetag name: $typetag -->\n";
            next;
        }
        $repeat{$typetag} = 1;
	dbg "typetag = $typetag";

        my $inheritString = getInheritString($typetag);

        my $source = $typeTagFiles{$typetag};
        my $oFile = basename($source);
        $oFile =~ s/$templatePath\///;
        if ($includePath) {$oFile =~ s/$includePath\///;}
        if ($focus{$typetag}){ $focusItem = " focus=\"$focus{$typetag}\""}
        else {undef $focusItem;}
        if ($focusItem){
#            my $spaces = ""; 
#            my $i;
#            for ($i=0; $i<$focus{$typetag}; $i++){
#                $spaces .= "   ";
#            }
            print $out " <typetag name=\"$typetag\" inherits=\"$inheritString\" source=\"$oFile:$typeTagLineNumber{$typetag}\" realpath=\"$source\" $focusItem>\n";
            my @akeys = keys(%propertyTypeTag);
            my @skeys = sort @akeys;
            my $property;
            foreach $property (@skeys){
                #property info here
                if ($propertyTypeTag{$property} eq $typetag) {
                    my $value = $propertyValues{$property};
                    $value =~ s/</&lt;/g;
                    $value =~ s/>/&gt;/g;
                    $value =~ s/"/&quot;/g;
                    $source = $propertySource{$property};
                    $oFile = basename($source);
                    $oFile =~ s/$templatePath\///;
                    if ($includePath){$oFile =~ s/$includePath\///;}
                    print $out "  <property name=\"$property\" value=\"$propertyValues{$property}\" source=\"$oFile:$propertyLineNumber{$property}\" realpath=\"$source\"/>\n";
                }
            }
            print $out " </typetag>\n";
        }
    }

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
    dbg "printFilesXML $start\n";
    printFileOut(OUTPUT, $start, 0);
    printPropertiesXML(OUTPUT);
    printTypeTagsXML(OUTPUT);
    print OUTPUT "</structure-info>\n";
    close OUTPUT;
}

####################################################################################


### pending:

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

####################################################################################


sub getRawLine {
# Returns logical line.
    my $nextLine;
    my $rawline = <INPUT>;
    $referenceLineCount++;

#remove initial whitespace
    $rawline =~ s/^\s+//;

    trace("getRawLine, line: $referenceLineCount\n$rawline");

# zap embedded C comments:
    if ($rawline =~ m/\/\*.*\*\//){
        trace "zap embedded comment: $_";
        $rawline =~ s/\/\*.*\*\///g;
        trace "zap result: $rawline";
    }
#   If we have a C comment initiator, continue until terminator.
    my $startComment = 0;
    if ($rawline =~ /\/\*.*/){
	trace("startComment...");
        $startComment = 1;
        $rawline =~ s/\/\*.*//;
	trace("new rawline: $rawline");
        while ($startComment == 1){
            $nextLine = &getRawLine;
	    trace("next line: $nextLine");
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

####################################################################################
####################################################################################
sub main {
    STDOUT->autoflush(1);
# 1.
    my $start = processArguments;
    resolveMissingArguments($start);
    dbg "1 ok";
# 2.
    getIncludeFileArray($start);
    dbg "2 ok";
# 3. 
    getTypeTags;
    dbg "3 ok";
# 4.
    my $f;
    foreach $f (@files){&getProperties($f)}
    foreach $f (@files){&getPropertyValues($f)}
    dbg "4 ok";
# 5.
    if (defined $problemTypeTag){
        &markFocus($problemTypeTag, 1);
    } else {
	dbg "markFocus(): problemTypeTag is not defined";
    }
    dbg "5 ok";
####################
  
    printXML($start);

    exit 1;

}
####################################################################################
####################################################################################


if (not $ARGV[0]){
    print "Please specify target file.\n";
    &usage
}
&main;
exit 1;


