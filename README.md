<h1>KCrypt -- a file encrypter/obfuscator</h1> 

Version 6.0, March 2013

*Please note that all references in this document to 'Windows' are to*
*Windows 7 and earlier. I don't use Windows any more, and I have no idea*
*whether this utility can still be built for Windows. Although it is very*
*old, it still builds and runs fine on Linux.*/

<h2>What is this?</h2>

<i>KCrypt</i> 
is a straightforward file encrypter and obfuscator, intended to provide 
strong security for sensitive files. It encrypts files using a key (password),
making them incomprehensible until they are decrypted, using the same key. 
KCrypt can be used to secure any type of file on fixed or removeable
storage devices.  KCrypt also has
a number of other privacy-related functions, as described below. 
<i>KCrypt</i> is based on the IDEA cipher, which is open, very strong,
 and no longer subject to  patent restrictions in most countries.
<p/>
<i>KCrypt</i> is primarily a command-line utility; versions are
available for Linux, the native Windows command prompt, and Cygwin.
However, there is also a version with a rudimentary graphical user interface,
designed to be integrated with Windows Explorer -- more
on this later. One of the main goals of this new release is cross-platform
compatibility. Not only does the program operate the same way on
all the platforms on which it can be built, but the files encrypted
on one platform can be decrypted on another (see <b>Platform compatibility
notes</b> below).
<p/>
I originally wrote <i>KCrypt</i> to protect examination papers 
that I was preparing
on my PC from the prying eyes of students, and for other confidential
documents like letters of reference. Before I wrote this program, I would have
kept these documents on floppy disks locked in my desk, which was not very
convenient. These days, of course, desktop computers don't have floppy 
disk drives, and removing a hard disk at the end of each working day
is even less convenient. <i>KCrypt</i> works best when encrypted files
are kept together in a small number of directories, particularly
when the same key is used for all files. This allows bunches of files
to be encrypted and decrypted together in a batch.
<p/> 
<i>KCrypt</i>
processes files in place; that is, the encryption process replaces the
plaintext file with an encrypted version, removing all trace of the original
file, while the decryption process has the opposite effect. In encrypting, 
some trouble is
taken to ensure that no part of a plaintext file is left on disk or in the
computer's memory after encryption. Thus you can encrypt a file in reasonable
confidence
that the unecrypted version is gone forever (until you decrypt it). However,
a number of situations can subvert this security measure; these will
be described in more detail below.
<p/>
<i>KCrypt</i> can optionally rename the file to a random string of digits, so
that the original name is not visible. In this case, it 
provides a facility to list the original names of encrypted files,
provided you have appropriate password. <i>KCrypt</i> can also
decrypt files to standard output, so they can be piped into another
program.
<p/>
<i>KCrypt</i> can process large batches of files in one invocation, and knows
to avoid encrypting files that are already encrypted, or decrypting files that
are not encrypted. This means that whole directories of files can be
maintained in an encrypted state; it is easy to decrypt one or more files 
to work on them,
then ensure that all files are encrypted by running KCrypt on the 
whole directory.  
It will even process an entire directory tree, although 
this kind of recursion has explicitly to be enabled on the command line. 
<p/>
<i>KCrypt</i> is a simple program with no external dependencies. It should be
possible to build it on any platform that has a C compiler. There 
are no special installation requirements beyond copying the executable
file to a convenient place; it does
not need administrative privileges to run, and can even be installed
without them if necessary. The overall size of the 
program is only about 100 kB and
it uses negligible RAM at run-time. It's small enough that it should
be possible to keep a version -- even several versions for different
platforms -- on a memory stick along with the data it protects.


<h2>DISCLAIMER -- Please read this</h2>

An encryption program, by its very nature, turns useful files 
into incomprehensible
gibberish. It is to be hoped that decryption will restore them to a useful 
state;
but, if either encryption or decryption fails,
you will be left with useless files. 
As this would probably be a significant inconvenience, please test this
program very carefully on non-critical data before trusting it on important
files. I accept no responsibility whatever for undesired consequences of using
this software.  Although <i>KCrypt</i> has been available as a 
Windows program for over twenty years, and as a separate Linux
program for over ten years, this new, fully cross-platform,
version was essentially written from scratch. Please exercise the same
caution as for any other relatively new software.

<h2>Differences from previous versions</h2>

<i>KCrypt</i> was previously available as a shell extension (DLL) for
the Windows desktop, and as a command-line program for Linux. These
versions did not produce compatible files, as they were built using
very different compilers. This
version is different in a number of ways.

<ul>
<li>The Windows GUI version is no longer a shell extension. It 
is a self-contained application, designed to be integrated with the 
Explorer 'Send to' menu (instructions below). The problem with shell
extensions is that they can really only be created using Microsoft tools,
and one of the main goals of this new versions is cross-platform
compatibility. Despite being less tightly integrated with the desktop,
the new version operates in much the same way as the old one did.</li>
<li>This version of <i>KCrypt</i> does not advertise its existence
in files that it creates. A simple application of <code>strings</code>
will not reveal the software used to encrypt the file, or even that 
it is encrypted.</li>
<li>While version 6.0 uses the same encryption algorithm as previous
versions by default, it also offers a much faster (although weaker)
algorithm for situations where speed is more important than security.</li>
<li>Version 6.0 can be built for Cygwin under Windows, as well as 
Windows GUI and Windows console applications. The Cygwin version
is more-or-less identical to the Linux version in operation.</li>
<li>This version does not store the file modification time in 
the encrypted file, and restore it when the file is decrypted.
In practice, doing this interfered with backup arrangements that
relied on file timestamp.</li>
</ul>

<h2>Command-line examples</h2>

Here are some examples of the use of <i>KCrypt</i> at the command
prompt. For full details, please
see the <a href="kcrypt6.man.html">man page</a>. In general, you can
specify only a file or list of files, and the program will prompt
for everything else; but the use of specific command-line switches is
likely to be more convenient.
<p/>
<pre class="codeblock">
kcrypt6 -e docs/exams/december2013.doc
</pre>

Encrypt the specified file in place (that is, overwriting the plaintext
file with the encrypted version). Prompt for password.

<p/>
<pre class="codeblock">
kcrypt6 --recurse docs/exams 
</pre>

Prompt for what operation to carry out on the directory <code>docs/exams</code>,
then carry out that operation on the first 20 files found,
asking for passwords, confirmation, etc., 
as appropriate. The 20-file limit is a safety measure that applies to
recursive directory searches, and can be changed using 
the <code>--max-files</code> switch. 

<p/>
<pre class="codeblock">
kcrypt6 --max-files 100 --recurse -e docs/exams 
</pre>

Encrypt in place all the files in <code>docs/exams</code>, up to
a limit of 100 files. Prompt for
the password. Any files that are already encrypted (by <i>KCrypt</i>)
are skipped.

<p/>
<pre class="codeblock">
kcrypt6 -d * 
</pre>

Decrypt in place all the files in the current directory, and restore
the original names if the filenames were mangled by a previous
encrypt operation. 
Prompt for
the password. Any files that are not encrypted (by <i>KCrypt</i>)
are skipped.

<p/>
<pre class="codeblock">
kcrypt6 --xor -m private/letter.doc 
</pre>

Encrypt the specific file <code>letter.doc</code> and mangle its name, that
is, give it a random name in the same directory. Prompt for the
password.

<p/>
<pre class="codeblock">
kcrypt6 -l private/* 
</pre>

List the encryption status of all files in the <code>private</code>
directory. If a valid password is given when prompted, then 
<i>KCrypt</i> displays the original filename of the file -- if
its name has been mangled -- and some additional, internal
information. If you don't care about the details, but just want
to know which files are encrypted, you can do:

<pre class="codeblock">
kcrypt6 -l private/* --password "" 
</pre>

This is one of the few occasions where it is safe to specify
the (null) password on the command line (for a discussion of
this point, see below).

<p/>
<pre class="codeblock">
kcrypt6 -d docs/ --max-files 100 
</pre>
 
Decrypt and restore the original names of all encrypted files in
the <code>docs</code> directory, up to a limit of 100 files.

<p/>
<pre class="codeblock">
kcrypt6 -w *.txt --yes 
</pre>
 
Wipe (zero out and then delete) all files whose names end 
in <code>.txt</code>, and
assume 'yes' is the answer to all prompts.


<p/>
<pre class="codeblock">
kcrypt6 --cat creditcard.txt 
</pre>
 
Decrypts the specified file and prints the result to standard
output. No temporary files are created during the decryption process.

<h2>Installation</h2>

There is no specific installer for any platform -- <i>KCrypt</i>
is a single executable file called <code>kcrypt6</code> (or
<code>kcrypt6.exe</code> on Windows/Cygwin). You can install that
file any place where it easy to find, and is on the search path
used by the command shell. On a Unix system, 
<code>/usr/bin</code> is a good choice; on Windows nothing is as 
obvious, but <code>C:\windows</code> will probably serve.
The Windows GUI version is called <code>kcrypt6_gui.exe</code>.
Note that this is not a full GUI -- it is intended to be integrated
into Windows explorer (see below).

<h3>How to set up KCrypt to use with Windows Explorer</h3>

<i>KCrypt</i> can be used with Windows Explorer, by adding
it to the 'Send to' menu, which is accessible by right-clicking
on a file or folder. The contents of the Send to menu are,
among other things, the shortcuts found in a specific directory.
On Windows 7 this directory is 

<pre>
<code>C:\Users\[name]\AppData\Roaming\Microsoft\Windows\SendTo</code>
</pre>

On earlier versions it was 

<pre>
<code>C:\Users\[name]\SendTo</code>
</pre>

To set up <i>KCrypt</i>, find the <code>SendTo</code> directory,
right-click and choose 'New Shortcut'. For the shortcut target,
select the location of <code>kcrypt6_gui.exe</code>. For the
name, choose whatever you want to appear on the menu. Thereafter,
you should be able to select files from the Explorer,
and use 'Send to' to send them to KCrypt.
<p/>
Note that in GUI mode recursion is disabled, and you'll have to 
select specific files, or sets of files, to process. You can't
select a directory. This is because the risk of disaster is just
too high, given that <i>KCrypt</i> is not very diligent about asking
for confirmations. 
<p/>
The GUI operation of <i>KCrypt</i> is intentionally very similar
to the command-line, when the program is invoked with no
switches. That is, the program will ask what operation you want to
perform on the selected files, prompt for passwords, etc., just as
at the command prompt. 
 

<h2>Building from source</h2>

<i>KCrypt</i> is written in ANSI-standard C, and intended to be compiled
with GNU <code>gcc</code> version 3 or later. The choice of compiler
is significant, because issues such as data type sizes and structure
alignment are very significant to the encryption process. There's no guarantee
that a version built by one version of GCC will produce encrypted files
that can be read by a version built by a different version. However,
there seems to be reasonable compatibility between GNU <code>gcc</code>
versions 3 and 4, on a range of different platforms.
<p/>
Building the program should, in principle, be as simple as 
unpacking the source bundle and running
<code>make</code>; you might need to define the environment
variable CC to point to <code>gcc</code> if your system does not
have a default for this. If you build with MinGW on Windows, 
directives in the Makefile should cause both the command-line and
GUI versions to be built.

<h2>Technical notes</h2>

<h3>Algorithm</h3>

The standard algorthim provided by <i>KCrypt</i> is Lai and 
Massey's International Data Encryption Algorithm (IDEA).
Although nearly thirty years old, this algorithm has
proven very resilient in the field. There are theoretically
stronger algorithms, but cryptographic strength is not really
the limiting factor with a program of this sort (see
discussion below). 
<p/>
The IDEA block cipher is used in a cipher feedback mode (CFB), to
reduce the tendency of repeated patterns in the input data
to give repeated patterns in the cipher text.
<p/>
<i>KCrypt</i> can also use a simple XOR-based cipher, also
in CFB mode, for non-critical situations where speed is more
important that cryptographic strength. Even the simple XOR
cipher will probably defeat all but the most determined 
intruders.

<h3>Speed</h3>

On a reasonable desktop computer, expect encryption and decryption
to take about one minute per gigabyte with the IDEA algorithm under
Linux. The native Windows (console or GUI) version is about half
that fast, and the Cygwin version half as fast again, on the same
hardware. The relatively poor performance of Cygwin is probably 
attributable to the overheads created by the Posix compatibility layer.
<p/>
On a Motorola Xoom Android table, running at the command
prompt, the encryption speed is about one-tenth 
the speed of a desktop PC, that is, about two megabytes per second.
Perhaps not all that impressive, but faster than most
Java-based encryption apps for Android. 
<p/>
Broadly, the XOR algorithm is about twice as fast as IDEA. 

<h2>Platform compatibility notes</h2>

With care in compiler settings, it is relatively straightforward
to build versions of <i>KCrypt</i> that are file-compatible
between Linux, native Windows, and Cygwin on X86 hardware.
The program can be built for Linux on ARM systems, but no
guarantees can be given of file compatibility between different
hardware platforms, particularly when the CPUs have different
endian-ness.
<p/>
A problem arises when moving files between platforms that use 
different character encoding for filenames. 
Although 
encryption and decryption should work, mangled filenames may not be
restored correctly, because the stored filename may not be representible
in the new platform's character set. In fact, handling Unicode filenames
is generally problematic on Windows -- see 'General limitations' below.


<h2>Limitations</h2>


<h3>General limitations and points to watch</h3>

<i>KCrypt</i> is not very persistent in asking for confirmation of
user actions. It assumes that if you select 1,000 files and
click the 'Wipe' button, that's exactly what you mean to do.
On the command line you can specify the <code>--yes</code> switch
to remove even the minimal prompting that is normally done. 
<p/>

<i>KCrypt</i> can only encrypt files in place (that is, 
overwriting the original file with an encrypted version), or
to a new file with a random name in the same directory. By
design, it won't allow files to be encrypted or decrypted into
new files in other locations. 
<p/>
As a consequence of the point above, <i>KCrypt</i>
needs the user to have access rights to create new files in the directory
containing the files to be encrypted or decrypted. 
<p/>
This version of <i>KCrypt</i> is not backwards-compatible with
any older version. I hear from time to time that there are a 
few people still using the very first version of
<i>KCrypt</i> that I wrote for
Windows 95 back in 1998. The problem is that compilers change
over the years, not to mention operating systems, and I am
not able to create a version of <i>KCrypt</i> that will 
read these old files with modern tools. It's probably possible,
but there are only so many hours in the day.
<p/>
A potential problem arises when handling Unicode filenames on the native 
Windows (not Cygwin) platform. 
When
<i>KCrypt</i> stores the original filename in a mangled file,
it uses the exact name provided by the C runtime library, byte-for-byte.
On Windows, filenames are stored in UTF-16 format, but they are delivered
to the program by the command prompt in ANSI (8-bit) equivalents. 
Consequently, any file
whose name cannot be rendered using the ANSI character set will not
be restored correctly when its name is unmangled. Using the 
GUI version of <i>KCrypt</i> for Windows does not entirely solve the
problem, because ANSI filename conversion also happens in the 
interface between Windows and the C run-time library. Cygwin and
Linux should not have this problem, and Windows is only affected
for filenames that cannot be represented using the ANSI character set.
 


<h3>Cryptographic and security limitations</h3>

If you take the security of your data seriously, I recommend 
that you read this section very carefully. 
<p/>
KCrypt is designed to be convenient to use, while remaining 
reasonably secure. The IDEA algorithm it uses by default is still
thought to be effectively unbreakable, but there are many ways to
attack a program of this sort besides trying to break the cryptography. 
The biggest weakness is that, in order to prevent the user 
from accidentally trying to decrypt a file with the wrong password
-- which would be very destructive -- encrypted files generated
by <i>KCrypt</i> contain
both encrypted and plaintext versions of a marker string.
This allows the password to be tested easily by the software,
because only the correct password will decrypt the encrypted marker string
to match the plain-text version. But, of course, this set-up allows
<i>many</i> passwords to be tested very rapdily, particularly 
on a fast machine.
Any encryption software that provides a way for a computer (rather
than a human) to determine whether the password is correct or not 
is open
to dictionary-based brute-force attacks. Avoidance of
dictionary words or other 'guessable' passwords will go a long way
towards avoiding this sort of problem. However, it is important
to understand that strong crytography does not, by itself, lead 
to impenetrable security. There's no question that providing a method
to test whether the decryption key is correct has a detrimental effect
on security; but it also makes operation very much more convenient.  
<p/>
Any cryptographic scheme can be compromised by a knowledgeable person who has
access to your computer. For example, he or she could install a piece 
of software
that records every keystroke you type, and sends them somewhere over the
Internet.
A knowledgeable person could even replace the <i>KCrypt</i> program file 
with another
that looked just the same, but did something completely different. It might,
for example, implement an encryption scheme that looked good, but was weak
enough to crack, or responded to keys other than the one you know about. 
<i>KCrypt</i> is open-source, so you can, if you want, build it
yourself from source (for Linux you will probably have to).
<p/>
In general, <i>KCrypt</i> encrypts and decrypts to temporary
files, then deletes unnecessary files at the end of
the operation. 'Unnecessary' files in this case might include,
for example, the original plain-text file when encrypting.
The program attempts to block Ctrl+C and other interrupts, and
to warn the user that exiting the program is likely to lead to
inadequate cleanup of files. This blocking works better on some
platforms than on others; on all platforms there is likely to 
be <i>some</i> way for a user to stop the program in its tracks 
-- pulling the plug out, in the last resort. 
To make errant temporary files easier to control, <i>KCrypt</i> 
only ever creates them in the same directory as the
files it is working on.
<p/>
Using <i>KCrypt</i> with the <code>--cat</code> switch to 
display the contents of an encrypted file does not create any
temporary files of its own. However, if you pipe the output
into another application, then temporary files might get created as
part of the pipe process. This is particularly likely on Windows, 
because the operating system does not have a built-in pipe
mechanism, so it simulates one using temporary files.
Files created this way are particularly 
troublesome, because we can't be sure where they are. Of course, 
Windows will delete them eventually, but only if you don't
exit <i>KCrypt</i> ungracefully; and even then it won't wipe the 
contents before deleting. 
<p/>
<i>KCrypt</i> goes to some lengths to ensure that passwords are
not left lurking in memory, where an intruder with access to 
your computer might be able to run a memory scanner to 
locate them. However, particularly with the Windows GUI, it is
very difficult to be really sure that the information entered by
the user is not retained somewhere in RAM or swap. 
<p/>
Similar considerations apply to temporary files deleted by the program --
<i>KCrypt</i> attempts to wipe all temporary files clean, by zeroing 
out their contents from start to finish. However, the software has 
no control over the
low-level operation of the kernel and disk drivers, and it can offer
no guarantees that some recognizable data won't be left lurking 
on the disk somewhere. Some filesystems even make a point of
holding on to older versions of files, for the convenience of users
who might accidentally delete or modify them. 
<p/>
For convience, <i>KCrypt</i> will allow you to throw caution to
the wind and specify an encryption or decryption password
on the command line, rather than entering at the prompt. The security implications of such an action
hardly bear thinking about, and it would be unwise to do this
for any purposes other than testing. 

<h3>User interface limitations</h3>

<i>KCrypt</i> only provides English-language messages and
prompts.
<p/>
<i>KCrypt</i> attempts not to echo passwords as they are typed.
There are specific ways to do this on native Windows, Cygwin, 
and Linux platforms. On all platforms this interference with 
the terminal/console can cause problems if you're redirecting
the output from the program to another utility that alters
terminal settings 
(for example, using <code>kcrypt6 -- cat | more</code>).
<p/>
The Windows GUI version of <code>KCrypt</code> does not allow
the encryption algorithm to be changed (IDEA is always used),
and it requires files, rather than directories to be selected.
There are no limts, however, other than those imposed by the 
Windows desktop, on the number of files that can be selected.
<p/>
<b>Kcrypt</b> intentionally does not process hidden files, if
they are found in a directory search. However, if hidden
files are specifically selected, either on the command line
or through Windows Explorer, they are processed. The logic
here is that if the user specifically selects a file it is
intentional, but the user probably wouldn't want files being
encrypted and decrypted when they are not normally visible.
<p/>
The text 'OK', 'Error', and 'Skipped' in messages is highlighted in
colour if the output is to a terminal, unless the
<code>--no-color</code> switch is given. The Windows console
version does not support this colour highlighting, which is done
using direct output of ANSI escape codes. The Cygwin terminal, and
most Linux terminals, support these escape codes. Some other terminals
might not, but full support for all possibilities would require
use of the terminfo database and all the overheads associated with that.
 

<h2>Author and legal</h2>

KCrypt is maintained by Kevin Boone, and is open-source under the
terms of the GNU Public Licence, version 3.0. There is no warranty
of any kind. 
The IDEA algorithm was developed by Dr Xuejia Lai and 
Prof. James L Massey.
A description of the algorithm may be found in: 
Lai, X (1992) ``On the Design and Security of 
Block Ciphers'', ETH Series on Information Processing (ed. Massey, JL) 
Volume 1, Konstanz, Switzerland: Hartung-Gorre Verlag, ISBN 3-89191-573-X.
The legal status of the algorithm is not entirely certain, although 
the owners of the patent have always stated that they were happy
for it to be used free of charge for private purposes. The original
patents expired between 2011 and 2012 in most countries, but 
I cannot promise that it is patent-free in all jurisdictions.
 
