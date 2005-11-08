import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.image.*;
import java.awt.geom.*;
import java.net.URL;
import javax.swing.border.*;
import java.util.*;
import java.io.*;

public class ImageViewer extends JApplet
{
  private class zoomLevel {
    int x1, y1, x2, y2, zf;
    
    zoomLevel() {}
    zoomLevel(zoomLevel zl)
    {
      x1 = zl.x1;
      y1 = zl.y1;
      x2 = zl.x2;
      y2 = zl.y2;
      zf = zl.zf;
    }
  }
  
  private BufferedImage bi;
  private Graphics2D big;
  private MML myMML;
  private int iw, ih;
  private int selected = 0, imgId;
  private Image img;
  private PgmImage pgm = new PgmImage();
  private String cmdline = new String();
  private static String hostname;
  private static boolean isApplet = true;
  private boolean fullRefresh = false;
  private Point offset = new Point(0,0);
  private zoomLevel zl = new zoomLevel();
  private Rectangle rect = new Rectangle();
  private Stack zoomStack = new Stack();
  private static String j2kfilename;

  public int getX()      { return offset.x; }
  public int getY()      { return offset.y; }
  public int getWidth()  { return iw; }
  public int getHeight() { return ih; }
  
  public void destroy()
  {
  }
  
  public void zoomIn()
  {
    Dimension asz = this.getSize();
    int maxzf = 3;
    int coef = 1;
    int r;
    
    cmdline = 
      "/bin/sh get.sh " + j2kfilename + " " + iw
      + " " + ih + " " + rect.x + " " + rect.y + " "
      + rect.width + " " + rect.height;
    Exec.execPrint(cmdline);

    rect.x = rect.y = rect.width = rect.height = 0;

    img = pgm.open("out.pgm");
    
    iw = img.getWidth(this);
    ih = img.getHeight(this);
    bi = new BufferedImage(iw, ih, BufferedImage.TYPE_INT_RGB);
    big = bi.createGraphics();
    selected = 0;
    fullRefresh = true;
    repaint();
  }

  public void zoomOut()
  {
  }

  public void init()
  {
    String str;
    int port;

    imgId = 4;
    if (isApplet && (((hostname = this.getParameter("hostname")) == null)
		    || hostname.equals("")))
      hostname = "localhost";
    if (!isApplet || ((str = this.getParameter("cmdPort")) == null)) {
      port = 3000;
    } else {
      port = new Integer(str).intValue();
    }
    
    this.setSize(512, 512);
    Dimension asz = this.getSize();
    zl.x2 = asz.width;
    zl.y2 = asz.height;
    
    cmdline = 
      "/bin/sh get.sh " + j2kfilename + " " + asz.width
      + " " + asz.height + " " + zl.x1 + " " + zl.y1 + " "
      + zl.x2 + " " + zl.y2;
    Exec.execPrint(cmdline);
    img = pgm.open("out.pgm");
    
    iw = img.getWidth(this);
    ih = img.getHeight(this);
    
    setBackground(Color.black);
    bi = new BufferedImage(iw, ih, BufferedImage.TYPE_INT_RGB);
    big = bi.createGraphics();
    myMML = new MML(this);
    addMouseListener(myMML);
    addMouseMotionListener(myMML);
  }
  
  public void setSelected(int state)
  {
    if (state != selected) {
      selected = state;
      repaint();
    }
  }
  
  public boolean isInsideRect(int x, int y)
  {
    return rect.contains(x - offset.x, y - offset.y);
  }

  public void setRGeom(int x1, int y1, int x2, int y2)
  {
    rect.x = Math.min(x1,x2) - offset.x;
    rect.y = Math.min(y1,y2) - offset.y;
    rect.width = Math.abs(x2-x1);
    rect.height = Math.abs(y2-y1);
  }

  public void paint(Graphics g)
  {
    Graphics2D g2 = (Graphics2D) g;
    Dimension asz = this.getSize();

    if (fullRefresh) {
      g2.clearRect(0, 0, asz.width, asz.height);
      fullRefresh = false;
    }
    g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                        RenderingHints.VALUE_ANTIALIAS_ON);
    g2.setRenderingHint(RenderingHints.KEY_RENDERING,
                        RenderingHints.VALUE_RENDER_QUALITY);
    big.setColor(Color.black);
    offset.x = (int) (asz.width  - iw) / 2;
    offset.y = (int) (asz.height - ih) / 2;
    big.drawImage(img, 0, 0, this);
    big.setPaint(Color.red);
    if ((rect.width > 0) && (rect.height > 0))
      big.draw(rect);
    if (selected == 1)
      shadeExt(big, 0, 0, 0, 64);
    else if (selected == 2) {
      shadeExt(big, 0, 0, 0, 255);
      selected = 1;
    }
    g2.drawImage(bi, offset.x, offset.y, this);
  }

  private void shadeRect(Graphics2D g2, int r, int g, int b, int a)
  {
    g2.setPaint(new Color(r, g, b, a));
    g2.fillRect(rect.x + 1, rect.y + 1, rect.width - 1, rect.height - 1);
  }
  
  private void shadeExt(Graphics2D g2, int r, int g, int b, int a)
  {
    g2.setPaint(new Color(r, g, b, a));
    g2.fillRect(0, 0, iw, rect.y); /* _N_ */
    g2.fillRect(rect.x + rect.width + 1, rect.y,
    		iw - rect.x - rect.width - 1, rect.height + 1); /* E */
    g2.fillRect(0, rect.y, rect.x, rect.height + 1); /* W */
    g2.fillRect(0, rect.y + rect.height + 1,
    		iw, ih - rect.y - rect.height - 1); /* _S_ */
  }

  protected URL getURL(String filename)
  {
    URL codeBase = this.getCodeBase();
    URL url = null;

    try {
      url = new URL(codeBase, filename);
    } catch (java.net.MalformedURLException e) {
      System.out.println("Couldn't create image: badly specified URL");
      return null;
    }

    return url;
  }

  public static void main(String s[])
  {
    if (s.length > 0)
      j2kfilename = s[0];
    else
      j2kfilename = "girl";
      System.out.println(j2kfilename);
    isApplet = false;
    JFrame f = new JFrame("ImageViewer");
    f.addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent e) {System.exit(0);}
    });
    JApplet applet = new ImageViewer();
    f.getContentPane().add("Center", applet);
    applet.init();
    f.pack();
    f.setSize(new Dimension(550,550));
    f.show();
  }
}
