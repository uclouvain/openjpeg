/*
 * $Id$
 *
 * Copyright (c) 2002-2011, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2011, Professor Benoit Macq
 * Copyright (c) 2010-2011, Kaori Hagihara
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

import java.net.*;
import java.io.*;
import java.util.*;


public class JPIPHttpClient 
{    
    private String comURL;
    protected int fw, fh;
    protected int rx, ry;
    protected int rw, rh;
    protected String cid;
    
    public JPIPHttpClient( String URI)
    {
	comURL = URI + "?";
	fw = fh = -1;
	rx = ry = -1;
	rw = rh = -1;
	cid = null;
    }

    public int getFw(){ return fw;}
    public int getFh(){ return fh;}
    public int getRx(){ return rx;}
    public int getRy(){ return ry;}
    public int getRw(){ return rw;}
    public int getRh(){ return rh;}
    
    public byte[] requestViewWindow( int reqfw, int reqfh)
    {
	if( cid != null)
	    return requestViewWindow( reqfw, reqfh, cid);
	else
	    return null;
    }
  
    public byte[] requestViewWindow( int reqfw, int reqfh, int reqrx, int reqry, int reqrw, int reqrh)
    {
	if( cid != null)
	    return requestViewWindow( reqfw, reqfh, reqrx, reqry, reqrw, reqrh, cid);
	else
	    return null;
    }

    public byte[] requestViewWindow( int reqfw, int reqfh, String reqcid)
    {
	return requestViewWindow( null, reqfw, reqfh, -1, -1, -1, -1, reqcid, false);
    }

    public byte[] requestViewWindow( int reqfw, int reqfh, int reqrx, int reqry, int reqrw, int reqrh, String reqcid)
    {
	return requestViewWindow( null, reqfw, reqfh, reqrx, reqry, reqrw, reqrh, reqcid, false);
    }

    public byte[] requestViewWindow( String target, int reqfw, int reqfh)
    {
	return requestViewWindow( target, reqfw, reqfh, -1, -1, -1, -1, null, false);
    }
    
    public byte[] requestViewWindow( String target, int reqfw, int reqfh, boolean reqcnew)
    {
	if( cid == null) // 1 channel allocation only
	    return requestViewWindow( target, reqfw, reqfh, -1, -1, -1, -1, null, reqcnew);
	else
	    return null;
    }
    
    public byte[] requestViewWindow( String target, int reqfw, int reqfh, int reqrx, int reqry, int reqrw, int reqrh)
    {
	return requestViewWindow( target, reqfw, reqfh, reqrx, reqry, reqrw, reqrh, null, false);
    }

 
    public byte[] requestViewWindow( int reqfw, int reqfh, String reqcid, boolean reqcnew)
    {
	return requestViewWindow( null, reqfw, reqfh, -1, -1, -1, -1, reqcid, reqcnew);
    }
    
    public byte[] requestViewWindow( String target, 
					int reqfw, int reqfh, 
					int reqrx, int reqry, 
					int reqrw, int reqrh, 
					String reqcid, boolean reqcnew)
    {
	String urlstring = const_urlstring( target, reqfw, reqfh, reqrx, reqry, reqrw, reqrh, reqcid, reqcnew);
	return GETrequest( urlstring);
    }
    
    public byte[] requestXML()
    {
	String urlstring = comURL;
	
	if( cid == null)
	    return null;
	
	urlstring = urlstring.concat( "cid=" + cid);
	urlstring = urlstring.concat( "&metareq=[xml_]");
    
	return GETrequest( urlstring);
    }
  
    private byte[] GETrequest( String urlstring)
    {
	int buflen = 0;
	URL url = null;
	HttpURLConnection urlconn = null;
	byte[] jptstream = null;
    
	try{
	    url = new URL( urlstring);
        
	    System.err.println("Requesting: " + url);
      
	    urlconn = (HttpURLConnection)url.openConnection();
	    urlconn.setRequestMethod("GET");
	    urlconn.setInstanceFollowRedirects(false);
	    urlconn.connect();
      
	    Map<String,java.util.List<String>> headers = urlconn.getHeaderFields();
	    java.util.List<String> hvaluelist;

	    if(( hvaluelist = headers.get("JPIP-fsiz")) != null){
		String hvalueline = hvaluelist.get(0);
		fw = Integer.valueOf( hvalueline.substring( 0, hvalueline.indexOf(','))).intValue();
		fh = Integer.valueOf( hvalueline.substring( hvalueline.indexOf(',')+1 )).intValue();
	
		System.err.println("fw,fh: " + fw + "," + fh);
	    }
      
	    if(( hvaluelist = headers.get("JPIP-roff")) != null){
		String hvalueline = hvaluelist.get(0);
		rx = Integer.valueOf( hvalueline.substring( 0, hvalueline.indexOf(','))).intValue();
		ry = Integer.valueOf( hvalueline.substring( hvalueline.indexOf(',')+1 )).intValue();
		System.err.println("rx,ry: " + rx + "," + ry);
	    }
    
	    if(( hvaluelist = headers.get("JPIP-rsiz")) != null){
		String hvalueline = hvaluelist.get(0);
		rw = Integer.valueOf( hvalueline.substring( 0, hvalueline.indexOf(','))).intValue();
		rh = Integer.valueOf( hvalueline.substring( hvalueline.indexOf(',')+1 )).intValue();
		System.err.println("rw,rh: " + rw + "," + rh);
	    }
    
	    if(( hvaluelist = headers.get("JPIP-cnew")) != null){
		String hvalueline = hvaluelist.get(0);
		cid = hvalueline.substring( hvalueline.indexOf('=')+1, hvalueline.indexOf(','));
		System.err.println("cid: " + cid);
	    }

	    InputStream input = urlconn.getInputStream();
	    buflen = input.available();
	    
	    if( buflen > 0){
		ByteArrayOutputStream tmpstream = new ByteArrayOutputStream();
		byte[] buf = new byte[ 1024];
	
		System.err.println("reading jptstream...");
	
		int redlen;
		do{
		    redlen = input.read( buf);
	  
		    if( redlen == -1)
			break;
		    tmpstream.write( buf, 0, redlen);
		}while( redlen > 0);
      
		buflen = tmpstream.size();
	
		jptstream = tmpstream.toByteArray();
            
		tmpstream = null;
      
		System.err.println("jptlen: " + buflen);
		System.err.println("    succeeded");  
	    }
	    else{
		System.err.println("No new jptstream");
	    }
	    input.close();
	}
	catch ( MalformedURLException e){
	    e.printStackTrace();
	}
	catch ( ProtocolException e){
	    e.printStackTrace();
	}    
	catch( ClassCastException  e){
	    e.printStackTrace();
	}
	catch( NullPointerException e){
	    e.printStackTrace();
	}  
	catch( UnknownServiceException e){
	    e.printStackTrace();
	}
	catch ( IOException e){
	    e.printStackTrace();
	}

	urlconn.disconnect();     
        
	return jptstream;
    }
  
    private String const_urlstring( String target, 
				    int reqfw, int reqfh, 
				    int reqrx, int reqry, 
				    int reqrw, int reqrh, 
				    String reqcid, boolean reqcnew)
    {
	String urlstring = comURL;

	if( target != null){
	    if( !urlstring.endsWith("?"))
		urlstring = urlstring.concat( "&");
	    urlstring = urlstring.concat( "target=" + target);
	}
	if( reqfw != -1 && reqfh != -1){
	    if( !urlstring.endsWith("?"))
		urlstring = urlstring.concat( "&");
	    urlstring = urlstring.concat( "fsiz=" + reqfw + "," + reqfh);
	}
	if( reqrx != -1 && reqry != -1){
	    if( !urlstring.endsWith("?"))
		urlstring = urlstring.concat( "&");
	    urlstring = urlstring.concat( "roff=" + reqrx + "," + reqry);
	}
	if( reqrw != -1 && reqrh != -1){
	    if( !urlstring.endsWith("?"))
		urlstring = urlstring.concat( "&");
	    urlstring = urlstring.concat( "rsiz=" + reqrw + "," + reqrh);
	}
	if( reqcid != null){
	    if( !urlstring.endsWith("?"))
		urlstring = urlstring.concat( "&");
	    urlstring = urlstring.concat( "cid=" + reqcid);
	}
	if( reqcnew){
	    if( !urlstring.endsWith("?"))
		urlstring = urlstring.concat( "&");
	    urlstring = urlstring.concat( "cnew=http");
	}
	return urlstring;
    }
    
    public void closeChannel()
    {
	if( cid == null)
	    return;
      
	try{
	    URL url = new URL( comURL + "cclose=" + cid);
	    System.err.println( "closing cid: " + cid);
      
	    HttpURLConnection urlconn = (HttpURLConnection)url.openConnection();
	    urlconn.setRequestMethod("GET");
	    urlconn.setInstanceFollowRedirects(false);
	    urlconn.connect();
      
	    Map headers = urlconn.getHeaderFields();
     
	    urlconn.disconnect();
	} catch ( MalformedURLException e){
	    e.printStackTrace();
	} catch ( IOException e){
	    e.printStackTrace();
	}
    }    
}
