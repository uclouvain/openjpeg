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

import java.io.*;
import java.net.*;

public class ImgdecClient{

    public static PnmImage decode_jptstream( byte[] jptstream, String cid, int fw, int fh)
    {
	if( jptstream != null)
	    send_JPTstream( jptstream);
	return get_PNMstream( cid, fw, fh);
    }

    public static PnmImage decode_jptstream( byte[] jptstream, String j2kfilename, String cid, int fw, int fh)
    {
	send_JPTstream( jptstream, j2kfilename, cid);
	return get_PNMstream( cid, fw, fh);
    }
    
    public static void send_JPTstream( byte[] jptstream)
    {
	try{
	    Socket imgdecSocket = new Socket( "localhost", 5000);
	    DataOutputStream os = new DataOutputStream( imgdecSocket.getOutputStream());
	    DataInputStream  is = new DataInputStream( imgdecSocket.getInputStream());
      
	    System.err.println("Sending " + jptstream.length + "Data Bytes to decodingServer");
	    
	    os.writeBytes("JPT-stream\n");
	    os.writeBytes("version 1.0\n");
	    os.writeBytes( jptstream.length + "\n"); 
	    os.write( jptstream, 0, jptstream.length);
      
	    byte signal = is.readByte();
      
	    if( signal == 0)
		System.err.println("    failed");
	} catch (UnknownHostException e) {
	    System.err.println("Trying to connect to unknown host: " + e);
	} catch (IOException e) {
	    System.err.println("IOException: " + e);
	}
    }

    public static void send_JPTstream( byte[] jptstream, String j2kfilename, String cid)
    {
	try{
	    Socket imgdecSocket = new Socket( "localhost", 5000);
	    DataOutputStream os = new DataOutputStream( imgdecSocket.getOutputStream());
	    DataInputStream  is = new DataInputStream( imgdecSocket.getInputStream());
	    int length = 0;
	    
	    if( jptstream != null)
		length = jptstream.length;
	    
	    System.err.println("Sending " + length + "Data Bytes to decodingServer");
      
	    os.writeBytes("JPT-stream\n");
	    os.writeBytes("version 1.0\n");
	    os.writeBytes( j2kfilename + "\n");
	    os.writeBytes( cid + "\n");
	    os.writeBytes( length + "\n");
	    os.write( jptstream, 0, length);
      
	    byte signal = is.readByte();
      
	    if( signal == 0)
		System.err.println("    failed");
	} catch (UnknownHostException e) {
	    System.err.println("Trying to connect to unknown host: " + e);
	} catch (IOException e) {
	    System.err.println("IOException: " + e);
	}
    }

    public static PnmImage get_PNMstream( String cid, int fw, int fh)
    {
	PnmImage pnmstream = new PnmImage();
	try {
	    Socket imgdecSocket = new Socket( "localhost", 5000);
	    DataOutputStream os = new DataOutputStream( imgdecSocket.getOutputStream());
	    DataInputStream is = new DataInputStream( imgdecSocket.getInputStream());
	    byte []header = new byte[7];
	    
	    os.writeBytes("PNM request\n");
	    os.writeBytes( cid + "\n");
	    os.writeBytes( fw + "\n");
	    os.writeBytes( fh + "\n");

	    read_stream( is, header, 7);
            
	    if( header[0] == 80){
		// P5: gray, P6: color  
		byte magicknum = header[1];
		if( magicknum == 5 || magicknum == 6){
		    int length;
		    boolean iscolor = magicknum==6 ? true:false;
		    if( iscolor)
			pnmstream.channel = 3;
		    else
			pnmstream.channel = 1;
		    pnmstream.width  = (header[2]&0xff)<<8 | (header[3]&0xff);
		    pnmstream.height = (header[4]&0xff)<<8 | (header[5]&0xff);
		    int maxval = header[6]&0xff;
	  
		    if( maxval == 255){
			length = pnmstream.width*pnmstream.height*pnmstream.channel;
			pnmstream.data = new byte [ length];
			read_stream( is, pnmstream.data, length);
		    }
		    else
			System.err.println("Error in get_PNMstream(), only 255 is accepted");
		}
		else
		    System.err.println("Error in get_PNMstream(), wrong magick number" + header[1]);
	    }
	    else
		System.err.println("Error in get_PNMstream(), Not starting with P");
	    os.close();
	    is.close();
	    imgdecSocket.close();
	} catch (UnknownHostException e) {
	    System.err.println("Trying to connect to unknown host: " + e);
	} catch (IOException e) {
	    System.err.println("IOException: " + e);
	}
	return pnmstream;
    }

    public static byte [] get_XMLstream( String cid)
    {
	byte []xmldata = null;

	try{
	    Socket imgdecSocket = new Socket( "localhost", 5000);
	    DataOutputStream os = new DataOutputStream( imgdecSocket.getOutputStream());
	    DataInputStream is = new DataInputStream( imgdecSocket.getInputStream());
	    byte []header = new byte[5];
	    
	    os.writeBytes("XML request\n");
	    os.writeBytes( cid + "\n");
      
	    read_stream( is, header, 5);
	    
	    if( header[0] == 88 && header[1] == 77 && header[2] == 76){
		int length = (header[3]&0xff)<<8 | (header[4]&0xff);
	
		xmldata = new byte[ length];
		read_stream( is, xmldata, length);
	    }
	    else
		System.err.println("Error in get_XMLstream(), not starting with XML");
	} catch (UnknownHostException e) {
	    System.err.println("Trying to connect to unknown host: " + e);
	} catch (IOException e) {
	    System.err.println("IOException: " + e);
	}
	return xmldata;
    }

    public static String query_cid( String j2kfilename)
    {
	String cid = null;
	
	try{
	    Socket imgdecSocket = new Socket( "localhost", 5000);
	    DataOutputStream os = new DataOutputStream( imgdecSocket.getOutputStream());
	    DataInputStream is = new DataInputStream( imgdecSocket.getInputStream());
	    byte []header = new byte[4];

	    os.writeBytes("CID request\n");
	    os.writeBytes( j2kfilename + "\n");

	    read_stream( is, header, 4);
	    
	    if( header[0] == 67 && header[1] == 73 && header[2] == 68){
		int length = header[3]&0xff;

		if( length > 0){
		
		    byte []ciddata = new byte[ length];
		    read_stream( is, ciddata, length);
		    cid = new String( ciddata);
		}
	    }
	    else
		System.err.println("Error in query_cid(), not starting with CID");
	}
	catch (UnknownHostException e) {
	    System.err.println("Trying to connect to unknown host: " + e);
	} catch (IOException e) {
	    System.err.println("IOException: " + e);
	}

	return cid;	
    }
  
    public static void read_stream( DataInputStream is, byte []stream, int length)
    {
	int remlen = length;
	int off = 0;

	try{
	    while( remlen > 0){
		int redlen = is.read( stream, off, remlen);

		if( redlen == -1){
		    System.err.println("    failed to read_stream()");
		    break;
		}
		off += redlen;
		remlen -= redlen;
	    }
	} catch (IOException e) {
	    System.err.println("IOException: " + e);
	}
    }

    public static void destroy_cid( String cid)
    {
	try{
	    Socket imgdecSocket = new Socket( "localhost", 5000);
	    DataOutputStream os = new DataOutputStream( imgdecSocket.getOutputStream());
	    DataInputStream  is = new DataInputStream( imgdecSocket.getInputStream());
	    
	    os.writeBytes("CID destroy\n");
	    os.writeBytes( cid + "\n");
	    
	    byte signal = is.readByte();
      
	    if( signal == 0)
		System.err.println("    failed");
	} catch (UnknownHostException e) {
	    System.err.println("Trying to connect to unknown host: " + e);
	} catch (IOException e) {
	    System.err.println("IOException: " + e);
	}
    }
}
