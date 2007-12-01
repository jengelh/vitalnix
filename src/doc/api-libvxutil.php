<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block"><i>libvxutil</i> features function commonly used all over the
Vitalnix code base.</p>

<h1>Function overview</h1>

<p class="code"><code>
<b>#</b>include &lt;vitalnix/libvxutil/libvxutil.h&gt;<br />
<br />
<b>char *</b>vxutil_azstr(<b>const char *</b>string);<br />
<b>int</b> vxutil_cryptpw(<b>const char *</b>key, <b>const char *</b>salt, <b>int</b> method, <b>char **</b>result);<br />
<b>int</b> vxutil_genpw(<b>char *</b>dest, <b>int</b> size, <b>unsigned int</b> flags);<br />
<b>int</b> vxutil_have_display(<b>void</b>);<br />
<b>unsigned int</b> vxutil_now_iday(<b>void</b>);<br />
<b>int</b> vxutil_only_digits(<b>const char *</b>s);<br />
<b>char *</b>vxutil_propose_home(<b>char *</b>dest, <b>size_t</b> size, <b>const char *</b>base, <b>const char *</b>username, <b>unsigned int</b> level);<br />
<b>char *</b>vxutil_propose_lname(<b>char *</b>dest, <b>size_t</b> size, <b>const char *</b>surname, <b>const char *</b>first_name);<br />
<b>char *</b>vxutil_quote(<b>const char *</b>str, <b>unsigned int</b> dbl, <b>char **</b>fmp);<br />
<b>int</b> vxutil_replace_run(<b>const char *</b>command, <b>const struct</b> HXoption <b>*</b>map);<br />
<b>char *</b>vxutil_slurp_file(<b>const char *</b>filename);<br />
<b>int</b> vxutil_string_iday(<b>const char *</b>date);<br />
<b>int</b> vxutil_string_xday(<b>const char *</b>date);<br />
<b>int</b> vxutil_valid_username(<b>const char *</b>username);<br />
<b>char *</b>vxutil_vx3uuid(<b>const char *</b>full_name, <b>long</b> iday);
</code></p>

<h2>vxutil_azstr</h2>

<p class="block">If <code>string</code> is <code>NULL</code>, returns a pointer
to a constant empty string (""), otherwise returns <code>string</code>.</p>

<h2>vxutil_cryptpw</h2>

<p class="block">Encrypts the plaintext in <code>key</code>, possibly utilizing
<code>salt</code> using the specified method (encryption) and puts the result
into <code>*rp</code>, which must be freed after usage. The method can be one
of:</p>

<table border="1">
  <tr>
    <td class="t1"><code>CRYPW_DES</code></td>
    <td class="t1">Use DES (56-bit, weak) (not available on Win32)</td>
  </tr>
  <tr>
    <td class="t2"><code>CRYPW_MD5</code></td>
    <td class="t2">Use MD5 (128-bit, not available on Win32)</td>
  </tr>
  <tr>
    <td class="t1"><code>CRYPW_BLOWFISH</code></td>
    <td class="t1">Use Blowfish encryption (448-bit, 5 rounds,
      recommended)</td>
  </tr>
</table>

<h2>vxutil_genpw</h2>

<p class="block">The <code>vxutil_genpw()</code> function will generate a
random password based on <code>flags</code> and will put the result into
<code>dest</code>, which is of length <code>size</code> (so the generated
password will be <code>size-1</code> long). The <code>flags</code> parameter is
a bitfield with one or more of the following bits:</p>

<table border="1">
  <tr>
    <td class="t1"><code>GENPW_O1DIGIT</code></td>
    <td class="t1">If the pseudo-random number generator happens to throw a
      number, use it for the password</td>
  </tr>
  <tr>
    <td class="t2"><code>GENPW_1DIGIT</code></td>
    <td class="t2">Always have a digit in the password</td>
  </tr>
  <tr>
    <td class="t1"><code>GENPW_O1CASE</code></td>
    <td class="t1">If the pseudo-random number generator happens to generate an
      uppercase character, use it for the password</td>
  </tr>
  <tr>
    <td class="t2"><code>GENPW_1CASE</code></td>
    <td class="t2">Always have an uppercase character in the password</td>
  </tr>
  <tr>
    <td class="t1"><code>GENPW_RAND</code></td>
    <td class="t1">Use a standard, non-phonetic generator</td>
  </tr>
  <tr>
    <td class="t2"><code>GENPW_JP</code></td>
    <td class="t2">Use a phonetic algorithm based on the Hepburn transcription
      of the Japanese alphabet</td>
  </tr>
  <tr>
    <td class="t1"><code>GENPW_ZH</code></td>
    <td class="t1">Use a phonetic algorithm based on (a reduced table of) the
      Pinyin transcriptions of Chinese hanzi (漢字)</td>
  </tr>
</table>

<p class="block">Although <code>GENPW_RAND</code>, <code>GENPW_JP</code> and
<code>GENPW_ZH</code> are both flags here, only one can be used at a time.
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
Windows environment is present by checking the <code>DISPLAY</code> environment
variable.</p>

<h2>vxutil_now_iday</h2>

<p class="block">Returns the current date measured in days from January 01
1970. This format is commonly used for password ageing monitoring.</p>

<h2>vxutil_only_digits</h2>

<p class="block">Returns non-zero if there are only digits (as determined by
<code>isdigit()</code>) in the <code>string</code>.</p>

<h2>vxutil_propose_home</h2>

<p class="block">Generates a home directory path, whose exact structure is
dependant on <code>level</code>. A larger level yields a deeper filesystem tree
depth, but reduces the number of concurrent inodes within a directory (this
reduces directory search time on legacy filesystems). Level 2 for example is
used by SourceForge's public servers.</p>

<table border="1">
  <tr>
    <td class="t1">Level 0</td>
    <td class="t1"><code>/home/USERNAME</code></td>
  </tr>
  <tr>
    <td class="t2">Level 1</td>
    <td class="t2"><code>/home/U/USERNAME</code></td>
  </tr>
  <tr>
    <td class="t1">Level 2</td>
    <td class="t1"><code>/home/U/US/USERNAME</code></td>
  </tr>
</table>

<p class="block">The result is put into <code>buf</code>. At most
<code>size-1</code> characters are written and the result is always
<code>'\0'</code>-terminated.</p>

<h2>vxutil_propose_lname</h2>

<p class="block">Creates a login name in the style of <code>FSSSSSS</code> out
of a real-world name. <code>surname</code> can be <code>NULL</code>, while
<code>firstname</code> must not be <code>NULL</code>. The two strings must be
UTF-8 encoded.</p>

<p class="block">There is special handling if the surname has multiple
parts&nbsp;-- let's take "van der Wies" as a fictional example. In this case,
we consider the last part as what we would like to have in the username.</p>

<h2>vxutil_quote</h2>

<p class="block">Returns a the string <code>str</code> in its escaped form
(without surround quotes, though), either single-quoted or double-quoted. If
<code>dbl</code> is zero, single-quoting is done, double-quoting on nonzero.
The function returns <code>NULL</code> on error, or a pointer to the escaped
string on success. If extra space was allocated, the pointer is stored in
<code>*fmp</code>, which is to be freed after usage. If no quoting was done,
because it was not needed, <code>*fmp</code> is set to <code>NULL</code>.</p>

<h2>vxutil_replace_run</h2>

<p class="block">Replaces every occurrence of a <code>%</code>-tag in the
<code>command</code> string by the defined value from <code>map</code> and then
run the expanded command using <code>system()</code>.</p>

<h2>vxutil_slurp_file</h2>

<p class="block">Reads in the file specified by <code>filename</code> in whole
and return a pointer to the newly allocated memory area, which should be freed
after usage. On error, <code>NULL</code> is returned and <code>errno</code>
will be set accordingly.</p>

<h2>vxutil_string_iday</h2>

<p class="block">Transforms the string <code>date</code>, which is of either
format of <code>DD.MM.YYYY</code>, <code>MM/DD/YYYY</code> or
<code>YYYY-MM-DD</code> into an integer representing the days since January 01
1970.</p>

<h2>vxutil_string_xday</h2>

<p class="block">Transforms the string <code>date</code>, which is of either
format of <code>DD.MM.YYYY</code>, <code>MM/DD/YYYY</code> or
<code>YYYY-MM-DD</code> into a BCD-style encoded integer representing the
date.</p>

<h2>vxutil_valid_username</h2>

<p class="block">Returns non-zero if the username does not contain any illegal
characters. Samba machine accounts are also handled.</p>

<h2>vxuuid_vx3</h2>

<p class="block">Generate a UUID from the string <code>full_name</code> and the
integer <code>iday</code>. This is used for Data Sources which come without an
UUID, so one is generated based on the (name, date) tuple we deem to be unique
within the Data Source. Returns a pointer to the newly allocated string
containing the UUID, or <code>NULL</code> on error.</p>

<?php include_once("Base-footer.php"); ?>
