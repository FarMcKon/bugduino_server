//package duinoServerFrontend;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.PushbackReader;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * @author mhorowitz
 * 
 */
public class Main {

	public static native int passInstruction( String instruction, int slot, byte[] arguments, int argumentsSize );

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		System.setProperty( "java.library.path", "/home/root" );
		System.loadLibrary( "javabugduino" );
		
		//System.out.println( "OUTPUT: " + passInstruction( "WRIT", 0, new byte[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }, 10 ));

		ServerSocket serverSocket = null;
		try {
			serverSocket = new ServerSocket( 8806 );

			while( true ){
				Socket clientSocket = serverSocket.accept();

				clientSocket.setSoTimeout( 5000 );
				
				InputStream rawSocketInput = clientSocket.getInputStream();
				OutputStream rawSocketOutput = clientSocket.getOutputStream();
				DataInputStream socketInput = new DataInputStream( rawSocketInput );
				DataOutputStream socketOutput = new DataOutputStream( rawSocketOutput );
								
				String instruction = socketInput.readUTF();
				/*
				{
					char[] instruction_chars = new char[4];
					for( int i = 0; i < 4; ++i ){
						instruction_chars[i] = socketInput.readChar();
					}
					instruction = new String( instruction_chars );
				}
				*/
												
				int slot = socketInput.readInt();
				int argumentsSize = socketInput.readInt();
				
				System.out.println( "Instruction: " + instruction + " Slot: " + slot + " ArgumentsSize: " + argumentsSize );
				
				try {
					Thread.sleep( 1000 );
				} catch (InterruptedException e) {
					System.out.println( "WARNING: Sleep interrupted - BUGduino might not have been programmed correctly." );
					e.printStackTrace();
				}
				
				byte[] arguments = new byte[argumentsSize];
				System.out.println( "Bytes read: " + socketInput.read( arguments, 0, argumentsSize ) + "(if this does not match ArgumentsSize, abort the operation and try again)" );
				
				int returnCode = passInstruction( instruction, slot, arguments, argumentsSize );

				//socketOutput.write( returnCode );
				//socketOutput.flush();
			}

		} catch (IOException e) {
			System.err.println( "ERROR: Problem getting data" );
			e.printStackTrace();
		}
	}
}
