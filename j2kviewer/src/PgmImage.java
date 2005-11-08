import java.awt.*;
import java.awt.image.*;
import java.net.*;
import java.io.*;
import java.util.regex.*;

class PgmImage extends Component
{
  private Socket s;
  private BufferedReader in;
  private int x, y;
  
  PgmImage()
  {
  }
  
  private String read()
  {
    try { return in.readLine(); }
    catch (IOException e) {
      e.printStackTrace();
      return null;
    }
  }
  
  public Image open(String filename)
  {
    String  str;
    Pattern pat;
    Matcher mat;
    int bytes, width, height, depth;
    FileInputStream fis;
    
    try {
      in  = new BufferedReader(
              new InputStreamReader(
	        fis = new FileInputStream(
		  new File(filename))));

      pat = Pattern.compile("^P5$");
      mat = pat.matcher(str = read());
      mat.matches();
      pat = Pattern.compile("^(\\d+) (\\d+)$");
      mat = pat.matcher(str = read());
      mat.matches();
      x = new Integer(mat.group(1)).intValue();
      y = new Integer(mat.group(2)).intValue();
      width  = x;
      height = y;
      depth  = 1;
      pat = Pattern.compile("^255$");
      mat = pat.matcher(str = read());
      mat.matches();
      bytes = x*y;
      char[] buf = new char[bytes];
      int r, offset = 0;
      while (bytes > 0) {
	try { r = in.read(buf, offset, bytes); offset += r; bytes -= r; }
	catch (IOException e) { e.printStackTrace(); }
      }
      int[] buf2 = new int[buf.length];
      if (depth == 3) {
	for (int i = 0; i < buf.length/3; ++i)
	  buf2[i] = 0xFF << 24 | buf[3*i] << 16 | buf[3*i+1] << 8 | buf[3*i+2];
      } else {
	for (int i = 0; i < buf.length; ++i)
	  buf2[i] = 0xFF << 24 | buf[i] << 16 | buf[i] << 8 | buf[i];
      }
      fis.close();
      return createImage(new MemoryImageSource(width, height, buf2, 0, width));
    } catch (IOException e) { e.printStackTrace(); }
    return null;
  }

  public void close()
  {
  }
  
  public boolean bye()
  {
    return true;
  }
  
  public int getXOffset()
  {
    return x;
  }
  
  public int getYOffset()
  {
    return y;
  }
}
