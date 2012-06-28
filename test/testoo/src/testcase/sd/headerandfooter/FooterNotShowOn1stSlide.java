/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



package testcase.sd.headerandfooter;

import static testlib.AppUtil.initApp;
import static testlib.UIMap.startcenter;
import static testlib.UIMap.*;

import java.io.File;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static org.openoffice.test.vcl.Tester.typeKeys;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.Log;
import testlib.UIMap;

public class FooterNotShowOn1stSlide {
	/**
	 * TestCapture helps us to do
	 * 1. Take a screenshot when failure occurs.
	 * 2. Collect extra data when OpenOffice crashes.
	 */	
	@Rule
	public Log LOG = new Log();
	
	/**
	 * initApp helps us to do 
	 * 1. Patch the OpenOffice to enable automation if necessary.
	 * 2. Start OpenOffice with automation enabled if necessary.
	 * 3. Reset OpenOffice to startcenter.
	 * 
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		initApp();
	}
	
	/**
	 * Test footer not show on the first slide. 
	 * @throws Exception
	 */
	@Test
	public void testFooterNotShowOn1stSlide() throws Exception{
		
		startcenter.menuItem("File->New->Presentation").select();
		sleep(2);
		
		for(int i=0; i<2;i++){
		SD_InsertPageButtonOnToolbar.click();
		sleep(1);
		}
		//pop up navigator panel
		impress.menuItem("View").select();
		if(!impress.menuItem("View->Navigator").isSelected()){
			impress.menuItem("View->Navigator").select();
			sleep(1);
		}
		
		//add header and footer
		impress.menuItem("View->Header and Footer...").select();
		sleep(2);
		SD_FooterTextOnSlide.check();
		sleep(1);
		SD_FooterTextOnSlideInput.setText("Footer Test");
		sleep(1);
		SD_FooterNotShowOn1stSlide.check();
		SD_ApplyToAllButtonOnSlideFooter.click();
		sleep(1);
							
		SD_SlidesPanel.focus();
		for(int j=0; j<2;j++){
		typeKeys("<up>");
		sleep(1);
		}

		impress.menuItem("View->Header and Footer...").select();
		sleep(1);
		assertEquals(true,SD_FooterNotShowOn1stSlide.isChecked());
		assertEquals(false,SD_FooterTextOnSlide.isChecked());

	}
	
}
