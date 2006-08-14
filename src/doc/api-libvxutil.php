<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block"><i>libvxutil</i> features function commonly used all over the
Vitalnix code base.</p>

<h1>Function overview</h1>

<p class="code"><tt>
<b>#</b>include &lt;libvxutil/libvxutil.h&gt;<br />
<br />
<b>char *</b>vxutil_azstr(<b>const char *</b>string);<br />
<b>int</b> vxutil_cryptpw(<b>const char *</b>key, <b>const char *</b>salt, <b>int</b> method, <b>char **</b>result);<br />
<b>int</b> vxutil_genpw(<b>char *</b>dest, <b>size_t</b> size, <b>long</b> flags);<br />
<b>int</b> vxutil_have_display(<b>void</b>);<br />
<b>long</b> vxutil_now_iday(<b>void</b>);<br />
<b>int</b> vxutil_only_digits(<b>const char *</b>s);<br />
<b>char *</b>vxutil_propose_home(<b>char *</b>dest, <b>size_t</b> size, <b>const char *</b>base, <b>const char *</b>username, <b>unsigned int</b> level);<br />
<b>char *</b>vxutil_propose_lname(<b>char *</b>dest, <b>size_t</b> size, <b>const char *</b>surname, <b>const char *</b>first_name);<br />
<b>char *</b>vxutil_quote(<b>const char *</b>str, <b>unsigned int</b> dbl, <b>char **</b>fmp);<br />
<b>int</b> vxutil_replace_run(<b>const char *</b>command, <b>const struct</b> HXoption <b>*</b>map);<br />
<b>char *</b>vxutil_slurp_file(<b>const char *</b>filename);<br />
<b>long</b> vxutil_string_iday(<b>const char *</b>date);<br />
<b>int</b> vxutil_valid_username(<b>const char *</b>username);<br />
<b>char *</b>vxutil_vx3uuid(<b>const char *</b>full_name, <b>long</b> iday);
</tt></p>

<h2>vxutil_azstr</h2>

<p class="block">If <tt>string</tt> is <tt>NULL</tt>, returns a pointer to a
constant empty string (""), otherwise returns <tt>string</tt>.</p>

<h2>vxutil_cryptpw</h2>

<p class="block">Encrypts the plaintext in <tt>key</tt>, possibly utilizing
<tt>salt</tt> using the specified method (encryption) and puts the result into
<tt>*rp</tt>, which must be freed after usage. The method can be one of:</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>CRYPW_DES</tt></td>
    <td class="t1">Use DES (56-bit, weak) (not available on Win32)</td>
  </tr>
  <tr>
    <td class="t2"><tt>CRYPW_MD5</tt></td>
    <td class="t2">Use MD5 (128-bit, not available on Win32)</td>
  </tr>
  <tr>
    <td class="t1"><tt>CRYPW_BLOWFISH</tt></td>
    <td class="t1">Use Blowfish encryption (448-bit, 5 rounds, recommended)</td>
  </tr>
</table>

<h2>vxutil_genpw</h2>

<p class="block">The <tt>vxutil_genpw()</tt> function will generate a random
password based on <tt>flags</tt> and will put the result into <tt>dest</tt>,
which is of length <tt>size</tt> (so the generated password will be
<tt>size-1</tt> long). The <tt>flags</tt> parameter is a bitfield with one or
more of the following bits:</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>GENPW_O1DIGIT</tt></td>
    <td class="t1">If the pseudo-random number generator happens to throw a
      number, use it for the password</td>
  </tr>
  <tr>
    <td class="t2"><tt>GENPW_1DIGIT</tt></td>
    <td class="t2">Always have a digit in the password</td>
  </tr>
  <tr>
    <td class="t1"><tt>GENPW_O1CASE</tt></td>
    <td class="t1">If the pseudo-random number generator happens to generate an
      uppercase character, use it for the password</td>
  </tr>
  <tr>
    <td class="t2"><tt>GENPW_1CASE</tt></td>
    <td class="t2">Always have an uppercase character in the password</td>
  </tr>
  <tr>
    <td class="t1"><tt>GENPW_RAND</tt></td>
    <td class="t1">Use a standard, non-phonetic generator</td>
  </tr>
  <tr>
    <td class="t2"><tt>GENPW_JP</tt></td>
    <td class="t2">Use a phonetic algorithm based on the Hepburn transcription
      of the Japanese alphabet</td>
  </tr>
  <tr>
    <td class="t1"><tt>GENPW_ZH</tt></td>
    <td class="t1">Use a phonetic algorithm based on (a reduced table of)
      the Pinyin transcriptions of Chinese hanzi (漢字)</td>
  </tr>
</table>

<p class="block">Although <tt>GENPW_RAND</tt>, <tt>GENPW_JP</tt> and
<tt>GENPW_ZH</tt> are both flags here, only one can be used at a time.
Specifying none or more than one is undefined behavior, but this implementation
will choose one.</p>

<p class="block">Vitalnix currently provides two different phonetic password
generation algorithms which will create passwords that can be pronounced and
(hopefully) remembered easily. The GENPW_JP and GENPW_ZH algorithms are very
easy, as they just pick random elements from a table of predefined values and
each currently has to obey only one extra rule to make it produce good
results.</p>

<h2>vxutil_have_display</h2>

<p class="block">This function can be used to determine if a graphical X
Windows environment is present by checking the <tt>DISPLAY</tt> environment
variable.</p>

<h2>vxutil_now_iday</h2>

<p class="block">Returns the current date measured in days from January 01
1970. This format is commonly used for password ageing monitoring.</p>

<h2>vxutil_only_digits</h2>

<p class="block">Returns non-zero if there are only digits (as determined by
<tt>isdigit()</tt>) in the <tt>string</tt>.</p>

<h2>vxutil_propose_home</h2>

<p class="block">Generates a home directory path, whose exact structure is
dependant on <tt>level</tt>. A larger level yields a deeper filesystem tree
depth, but reduces the number of concurrent inodes within a directory (this
reduces directory search time on legacy filesystems). Level 2 for example is
used by SourceForge's public servers.</p>

<table border="1" class="bordered">
  <tr>
    <td>Level 0</td>
    <td><tt>/home/USERNAME</tt></td>
  </tr>
  <tr>
    <td>Level 1</td>
    <td><tt>/home/U/USERNAME</tt></td>
  </tr>
  <tr>
    <td>Level 2</td>
    <td><tt>/home/U/US/USERNAME</tt></td>
  </tr>
</table>

<p class="block">The result is put into <tt>buf</tt>. At most <tt>size-1</tt>
characters are written and the result is always <tt>'\0'</tt>-terminated.</p>

<h2>vxutil_propose_lname</h2>

<p class="block">Creates a login name in the style of <tt>FSSSSSS</tt> out of a
real-world name. <tt>surname</tt> can be <tt>NULL</tt>, while
<tt>firstname</tt> must not be <tt>NULL</tt>. The two strings must be UTF-8
encoded.</p>

<p class="block">There is special handling if the surname has multiple
parts&nbsp;-- let's take "van der Wies" as a fictional example. In this case,
we consider the last part as what we would like to have in the username.</p>

<h2>vxutil_quote</h2>

<p class="block">Returns a the string <tt>str</tt> in its escaped form (without
surround quotes, though), either single-quoted or double-quoted. If
<tt>dbl</tt> is zero, single-quoting is done, double-quoting on nonzero. The
function returns <tt>NULL</tt> on error, or a pointer to the escaped string on
success. If extra space was allocated, the pointer is stored in <tt>*fmp</tt>,
which is to be freed after usage. If no quoting was done, because it was not
needed, <tt>*fmp</tt> is set to <tt>NULL</tt>.</p>

<h2>vxutil_replace_run</h2>

<p class="block">Replaces every occurrence of a <tt>%</tt>-tag in the
<tt>command</tt> string by the defined value from <tt>map</tt> and then run the
expanded command using <tt>system()</tt>.</p>

<h2>vxutil_slurp_file</h2>

<p class="block">Reads in the file specified by <tt>filename</tt> in whole and
return a pointer to the newly allocated memory area, which should be freed
after usage. On error, <tt>NULL</tt> is returned and <tt>errno</tt> will be set
accordingly.</p>

<h2>vxutil_string_iday</h2>

<p class="block">Transforms the string <tt>date</tt>, which is of either format
of <tt>DD.MM.YYYY</tt>, <tt>MM/DD/YYYY</tt> or <tt>YYYY-MM-DD</tt> into an
integer representing the days since January 01 1970.</p>

<h2>vxutil_valid_username</h2>

<p class="block">Returns non-zero if the username does not contain any illegal
characters. Samba machine accounts are also handled.</p>

<h2>vxuuid_vx3</h2>

<p class="block">Generate a UUID from the string <tt>full_name</tt> and the
integer <tt>iday</tt>. This is used for Data Sources which come without an
UUID, so one is generated based on the (name, date) tuple we deem to be unique
within the Data Source. Returns a pointer to the newly allocated string
containing the UUID, or <tt>NULL</tt> on error.</p>

<?php include_once("Base-footer.php"); ?>
