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

import java.awt.Image;

public class ImageManager extends JPIPHttpClient
{
    private PnmImage pnmimage;

    public ImageManager( String uri)
    {
	super( uri);
	pnmimage = null;
    }
    
    public int getOrigWidth(){ return pnmimage.width;}
    public int getOrigHeight(){ return pnmimage.height;}
    
    public Image getImage( String j2kfilename, int reqfw, int reqfh, boolean reqcnew)
    {
	System.err.println();
	
	String refcid = null;
	byte[] jpipstream;
	
	if( reqcnew)
	    refcid = ImgdecClient.query_cid( j2kfilename);
	    
	if( refcid == null){
	    String reftid = ImgdecClient.query_tid( j2kfilename);
	    if( reftid == null)
		jpipstream = super.requestViewWindow( j2kfilename, reqfw, reqfh, reqcnew);
	    else
		jpipstream = super.requestViewWindow( j2kfilename, reftid, reqfw, reqfh, reqcnew);
	}
	else
	    jpipstream = super.requestViewWindow( reqfw, reqfh, refcid, reqcnew);
	
	System.err.println( "decoding to PNM image");
	pnmimage = ImgdecClient.decode_jpipstream( jpipstream, j2kfilename, tid, cid, fw, fh);
	System.err.println( "     done");
	
	return pnmimage.createROIImage( rx, ry, rw, rh);
    }
    
    public Image getImage( int reqfw, int reqfh, int reqrx, int reqry, int reqrw, int reqrh)
    {
	System.err.println();
	
	byte[] jpipstream = super.requestViewWindow( reqfw, reqfh, reqrx, reqry, reqrw, reqrh);

	System.err.println( "decoding to PNM image");
	pnmimage = ImgdecClient.decode_jpipstream( jpipstream, tid, cid, fw, fh);
	System.err.println( "     done");
	
	return pnmimage.createROIImage( rx, ry, rw, rh);
    }
    
    public byte[] getXML()
    {
	System.err.println();
	
	byte []xmldata = null;
	byte[] jpipstream = super.requestXML();
	
	if( jpipstream != null){
	    ImgdecClient.send_JPIPstream( jpipstream);
      
	    xmldata = ImgdecClient.get_XMLstream( cid);    
	}
	return xmldata;
    }
    public void closeChannel()
    {
	if( cid != null){
	    ImgdecClient.destroy_cid( cid);
	    super.closeChannel();
	}
    }
}