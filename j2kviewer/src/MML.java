import java.awt.event.*;

class MML implements MouseMotionListener, MouseListener
{
  public void mouseExited(MouseEvent e) {}
  public void mouseEntered(MouseEvent e) {}
  public void mouseClicked(MouseEvent e) {}
  
  private ImageViewer applet;
  private int x1, y1, x2, y2, zf, btn;
  private boolean zoomrq;
  
  public MML(ImageViewer iv)
  {
    x1 = y1 = -1;
    applet = iv;
    zoomrq = false;
    zf = 0;
  }
  
  private boolean isInside(int x, int y)
  {
    x -= applet.getX();
    y -= applet.getY();
    return (x >= 0) && (x < applet.getWidth())
        && (y >= 0) && (y < applet.getHeight());
  }

  public void mousePressed(MouseEvent e)
  {
    btn = e.getButton();
    if (applet.isInsideRect(e.getX(), e.getY())) {
      applet.setSelected(2);
      applet.repaint();
      zoomrq = true;
    } else {
      applet.setRGeom(0, 0, 0, 0);
      applet.setSelected(0);
      applet.repaint();
      x1 = y1 = -1;
    }
  }
  
  public void mouseReleased(MouseEvent e)
  {
    if (zoomrq && (e.getButton() == 1)) {
      applet.zoomIn();
      zoomrq = false;
    } else if (e.getButton() == 3) {
      applet.zoomOut();
      zoomrq = false;
    }
  }

  public void mouseMoved(MouseEvent e)
  {
    applet.setSelected(applet.isInsideRect(e.getX(), e.getY()) ? 1 : 0);
  }
  
  public void mouseDragged(MouseEvent e)
  {
    String str;
    
    if (btn == 1) {
      x2 = e.getX();
      y2 = e.getY();

      applet.setSelected(0);
      zoomrq = false;

      if (isInside(x2, y2)) {
	str = "[IN ]";
	if (x1 == -1) {
          x1 = x2;
	  y1 = y2;
	} else {
          applet.setRGeom(x1, y1, x2, y2);
	  applet.repaint();
	}
      } else {
	str = "[OUT]";
      }
    }    
  }
}
