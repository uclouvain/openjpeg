import java.io.*;

// This appears in Core Web Programming from
// Prentice Hall Publishers, and may be freely used
// or adapted. 1997 Marty Hall, hall@apl.jhu.edu.

/** A class that eases the pain of running external
 *  processes from applications.
 *  Lets you run a program three ways:
 *  <OL>
 *     <LI><B>exec</B>: Execute the command, returning
 *         immediately even if the command is still
 *         running. This would be appropriate
 *         for printing a file.
 *     <LI><B>execWait</B>: Execute the command, but
 *         don't return until the command finishes.
 *         This would be appropriate for
 *         sequential commands where the first depends
 *         on the second having finished (e.g.
 *         <CODE>javac</CODE> followed by
 *         <CODE>java</CODE>).
 *     <LI><B>execPrint</B>: Execute the command and
 *         print the output. This would be appropriate
 *         for the UNIX command <CODE>ls</CODE>.
 *  </OL>
 *  Note that the PATH is not taken into account,
 *  so  you must specify the <B>full</B> pathname to
 *  the command, and shell builtin commands
 *  will not work. For instance, on Unix the above
 *  three examples might look like:
 *  <OL>
 *    <LI><PRE>Exec.exec("/usr/ucb/lpr Some-File");</PRE>
 *    <LI><PRE>
 *        Exec.execWait("/usr/local/bin/javac Foo.java");
 *        Exec.execWait("/usr/local/bin/java Foo");
 *        </PRE>
 *    <LI><PRE>Exec.execPrint("/usr/bin/ls -al");</PRE>
 *  </OL>
 *
 * @author Marty Hall
 *  (<A HREF="mailto:hall@apl.jhu.edu">
 *   hall@apl.jhu.edu</A>)
 * @version 1.0 1997
 */

public class Exec {
  //----------------------------------------------------
  
  private static boolean verbose = true;

  /** Determines if the Exec class should print which
   *  commands are being executed, and print error
   *  messages if a problem is found. Default is true.
   *
   * @param verboseFlag true: print messages.
   *        false: don't.
   */
  
  public static void setVerbose(boolean verboseFlag) {
    verbose = verboseFlag;
  }

  /** Will Exec print status messages? */
  
  public static boolean getVerbose() {
    return(verbose);
  }
  
  //----------------------------------------------------
  /** Starts a process to execute the command. Returns
   *  immediately, even if the new process is still
   *  running.
   *
   * @param command The <B>full</B> pathname of the
   *        command to be executed. No shell builtins
   *        (e.g. "cd") or shell meta-chars (e.g. ">")
   *        allowed.
   * @return false if a problem is known to occur, but
   *         since this returns immediately, problems
   *         aren't usually found in time.
   *         Returns true otherwise.
   */
  
  public static boolean exec(String command) {
    return(exec(command, false, false));
  }
  
  //----------------------------------------------------
  /** Starts a process to execute the command. Waits
   *  for the process to finish before returning.
   *
   * @param command The <B>full</B> pathname of the
   *        command to be executed. No shell builtins
   *        or shell meta-chars allowed.
   * @return false if a problem is known to occur,
   *         either due to an exception or from the
   *         subprocess returning a non-zero value.
   *         Returns true otherwise.
   */
  
  public static boolean execWait(String command) {
    return(exec(command, false, true));
  }
  
  //----------------------------------------------------
  /** Starts a process to execute the command. Prints
   *  all output the command gives.
   *
   * @param command The <B>full</B> pathname of the
   *        command to be executed. No shell builtins
   *        or shell meta-chars allowed.
   * @return false if a problem is known to occur,
   *         either due to an exception or from the
   *         subprocess returning a non-zero value.
   *         Returns true otherwise.
   */
  
  public static boolean execPrint(String command) {
    return(exec(command, true, false));
  }
  
  //----------------------------------------------------
  // This creates a Process object via
  // Runtime.getRuntime.exec(). Depending on the
  // flags, it may call waitFor on the process
  // to avoid continuing until the process terminates,
  // or open an input stream from the process to read
  // the results.

  private static boolean exec(String command,
                              boolean printResults,
                              boolean wait) {
    if (verbose) {
      printSeparator();
      System.out.println("Executing '" + command + "'.");
    }
    try {
      // Start running command, returning immediately.
      Process p  = Runtime.getRuntime().exec(command);
      
      // Print the output. Since we read until
      // there is no more input, this causes us
      // to wait until the process is completed
      if(printResults) {
        BufferedInputStream buffer =
          new BufferedInputStream(p.getInputStream());
        DataInputStream commandResult =
          new DataInputStream(buffer);
        String s = null;
        try {
          while ((s = commandResult.readLine()) != null)
            System.out.println("Output: " + s);
          commandResult.close();
          if (p.exitValue() != 0) {
            if (verbose)
              printError(command +
                         " -- p.exitValue() != 0");
            return(false);
          }
        // Ignore read errors; they mean process is done
        } catch (Exception e) {}
        
      // If you don't print the results, then you
      // need to call waitFor to stop until the process
      // is completed
      } else if (wait) {
        try {
          System.out.println(" ");
          int returnVal = p.waitFor();
          if (returnVal != 0) {
            if (verbose)
              printError(command);
            return(false);
          }
        } catch (Exception e) {
          if (verbose)
            printError(command, e);
          return(false);
        }
      }
    } catch (Exception e) {
      if (verbose)
        printError(command, e);
      return(false);
    }
    return(true);
  }
  
  //----------------------------------------------------

  private static void printError(String command,
                                 Exception e) {
    System.out.println("Error doing exec(" +
                       command + "): " + e.getMessage());
    System.out.println("Did you specify the full " +
                       "pathname?");
  }

  private static void printError(String command) {
    System.out.println("Error executing '" +
                       command + "'.");
  }
    
  //----------------------------------------------------

  private static void printSeparator() {
    System.out.println
      ("==============================================");
  }
  
  //----------------------------------------------------
}
