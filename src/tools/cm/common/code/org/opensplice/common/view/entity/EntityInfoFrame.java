/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.common.view.entity;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Event;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;

import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.WindowConstants;
import javax.swing.table.TableColumnModel;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataReader;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.Participant;
import org.opensplice.cm.Partition;
import org.opensplice.cm.Publisher;
import org.opensplice.cm.Query;
import org.opensplice.cm.Queue;
import org.opensplice.cm.Reader;
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Writer;
import org.opensplice.common.CommonException;
import org.opensplice.common.controller.EntityInfoListener;
import org.opensplice.common.controller.MainWindowOpener;
import org.opensplice.common.model.ModelListener;
import org.opensplice.common.model.ModelRegister;
import org.opensplice.common.model.table.EntityAttributeTableModel;
import org.opensplice.common.model.table.status.EntityStatusTableModel;
import org.opensplice.common.model.table.status.PartitionStatusTableModel;
import org.opensplice.common.model.table.status.ReaderStatusTableModel;
import org.opensplice.common.model.table.status.SubscriberStatusTableModel;
import org.opensplice.common.model.table.status.TopicStatusTableModel;
import org.opensplice.common.model.table.status.WriterStatusTableModel;
import org.opensplice.common.util.Config;
import org.opensplice.common.view.StatusPanel;
import org.opensplice.common.view.table.QoSTable;
import org.opensplice.common.view.table.StatisticsTable;


/**
 * Represents an interal frame that is capable of resolving and displaying
 * Entity information. Its responsibility is to:
 * - Resolve and display Entity attributes as well as their values.
 * - Resolve and display Entity status.
 * - Resolve and display Entity QoS.
 * - Allow user to change Entity QoS.
 * 
 * @date Oct 1, 2004
 */
public class EntityInfoFrame extends JFrame implements ActionListener, ModelListener {
    private Entity entity = null;
	private JPanel rootPanel = null;
	private JTabbedPane tabbedPane = null;
	private JScrollPane attributesScrollPane = null;
	private JTable attributeTable = null;
	private JScrollPane statusScrollPane = null;
	private JTable statusTable = null;
    private QoSTable qosTable = null;
	private JPanel qosPanel = null;
    private EntityInfoPane dataTypePane = null;
    private JScrollPane statisticsPane = null;
    private StatisticsTable statisticsTable = null;
    private Timer updateTimer = null;
    private JScrollPane dataTypeScrollPane = null;
    private JScrollPane qosScrollPane = null;
    private ModelRegister register = null;
    private StatusPanel statusPanel = null;
    private boolean iconified = false;
    private boolean closed = false;
    private EntityInfoListener controller = null;
    private JMenuBar infoMenuBar = null;
    private JMenu fileMenu = null;
    private JMenu editMenu = null;
    private JMenu viewMenu = null;
    private JMenuItem viewMainItem = null;
    private JMenuItem refreshItem = null;
    private JRadioButtonMenuItem contentTextItem = null;
    private JRadioButtonMenuItem contentHtmlItem = null;
    private JMenuItem closeItem = null;
    private JFrame parent = null;
    private JPanel buttonPanel = null;
    private JButton getQoSButton = null;
    private JButton setQoSButton = null;
    private String type = null;
    
	/**
	 * This is the default constructor that creates a frame that displays
     * information about the supplied Entity. Information about changes 
     * in the view are communicated to the supplied ModelRegister.
     * 
     * @param _entity The Entity to display.
     * @param _register The ModelRegister where to communicate changes to.
	 */
	public EntityInfoFrame(Entity _entity, ModelRegister _register, JFrame _parent) {
		super(_entity.toStringExtended() + " | Entity info");
        parent = _parent;
        register = _register;
        entity = _entity;
		initialize();
	}
    
    /**
     * Provides access to the Entity of the frame.
     * 
     * @return The Entity of the frame.
     */
    public Entity getEntity(){
        return entity;
    }
    
    
    /**
     * Sets the update delay of the frame. The selected tab determines which 
     * part of the information is being updated.
     * 
     * @param millis The update delay in milliseconds. If the supplied delay
     *               is smaller then 0, automatic updating is disabled.
     */
    public synchronized void setUpdateDelay(int millis){
        if(millis > 0){
            if(updateTimer != null){
                if(updateTimer.isRunning()){
                    updateTimer.stop();
                }
            }
            updateTimer = new Timer(millis, this);
            updateTimer.setRepeats(false);
            updateTimer.start();
            this.fireFrameChanged("update_delay");
        } else if(updateTimer != null){
            if(updateTimer.isRunning()){
                updateTimer.stop();
            }
            updateTimer = null;
            this.fireFrameChanged("update_delay");
        }
    }
    
    /**
     * Changes the content type of the EntityInfoPane that is displaying the
     * data type of the Entity.
     * 
     * @param contentType The content type to set. Supported content types are
     *                    'text/plain' and 'text/html'.
     * @return true if applying of the data type succeeded, false otherwise.
     */
    public boolean setDataTypeContentType(String contentType){
        boolean result = false;
        
        if((dataTypePane != null) && (contentType != null)){
            result = dataTypePane.setViewType(contentType);
            
        }
        type = contentType;
        selectContentType();
        return result;
    }
    
    /**
     * This method initializes attributeTable   
     *  
     * @return Table that holds the attributes of the Entity and its current
     *         values.  
     */    
    public JTable getAttributeTable() {
        if (attributeTable == null) {
            attributeTable = new JTable(new EntityAttributeTableModel(entity));
            attributeTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
            
            TableColumnModel cModel = attributeTable.getColumnModel();
            cModel.getColumn(0).setPreferredWidth(100);
            cModel.getColumn(0).setMaxWidth(100);
            attributeTable.setPreferredScrollableViewportSize(new java.awt.Dimension(300,150));
            attributeTable.getTableHeader().setReorderingAllowed(false);
        }
        return attributeTable;
    }
    
    /**
     * Handles events that are triggered by the update timer. The selected tab
     * will be updated and the timer restarted.
     * 
     * @param e The event that occurred.
     */
    @Override
    public void actionPerformed(ActionEvent e) {
        boolean success;
        
        if(e.getSource().equals(updateTimer)){
            success = false;
                        
            if(!iconified && !closed){
                if(tabbedPane.getSelectedComponent().equals(statusScrollPane)){
                    try {
                        success = ((EntityStatusTableModel)(this.getStatusTable().getModel())).update();
                    } catch (CommonException e1) {
                        success = false;
                    }
                } else if(tabbedPane.getSelectedComponent().equals(attributesScrollPane)){
                    success = ((EntityAttributeTableModel)(this.getAttributeTable().getModel())).update();
                } else if(tabbedPane.getSelectedComponent().equals(qosPanel)){
                    success = true;
                } else if(tabbedPane.getSelectedComponent().equals(statisticsPane)){
                    try{
                        success = this.getStatisticsTable().update();
                    } catch(CommonException ce){
                        success = false;
                    }
                } else {
                    success = true;
                }
                
                if(!success){
                    if(updateTimer != null){
                        if(updateTimer.isRunning()){
                            updateTimer.stop();
                        }
                        updateTimer = null;
                    }
                    this.fireFrameChanged("entity_freed");
                    this.dispose();
                } else if(!closed){
                    if(updateTimer.isRunning()){
                        updateTimer.restart();
                    } else {
                        updateTimer.start();
                    }
                } else {
                    if(updateTimer.isRunning()){
                        updateTimer.stop();
                        updateTimer = null;
                    }
                }
            }
        }
    }
    
    /**
     * Updates the information in the selected tab.
     * 
     * @param msg The update message.
     */
    @Override
    public void update(String msg){
        if(msg != null){
            if("tabbed_pane_update".equals(msg)){
                Component selected = tabbedPane.getSelectedComponent();
                
                if(selected.equals(statusScrollPane)){
                    refreshItem.setEnabled(false);
                    if(statusTable == null){
                        setCursor(new Cursor(Cursor.WAIT_CURSOR));
                        setStatus("Initializing status pane...", true, true);
                        
                        Runnable worker = new Runnable(){
                            @Override
                            public void run(){
                                try {
                                    statusScrollPane.setViewportView(getStatusTable());
                                    setStatus("Status pane initialized", false, false);
                                    setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                                } catch (CommonException e) {
                                    closed = true;
                                    fireFrameChanged("entity_freed");
                                    dispose();
                                }
                            }
                        };
                        SwingUtilities.invokeLater(worker);    
                    }
                    refreshItem.setEnabled(true);
                } else if(selected.equals(dataTypeScrollPane)){
                    refreshItem.setEnabled(false);
                    
                    if(dataTypePane == null){
                        setCursor(new Cursor(Cursor.WAIT_CURSOR));
                        setStatus("Initializing data type pane...", true, true);
                        
                        Runnable worker = new Runnable(){
                            @Override
                            public void run(){
                                dataTypeScrollPane.setViewportView(getDataTypePane());
                                setStatus("Data type pane initialized", false, false);
                                setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                            }
                        };
                        SwingUtilities.invokeLater(worker);
                    }
                } else if(selected.equals(qosPanel)){
                    refreshItem.setEnabled(true);
                    
                    if(qosTable == null){
                        setCursor(new Cursor(Cursor.WAIT_CURSOR));
                        setStatus("Initializing QoS pane...", true, true);
                        
                        Runnable worker = new Runnable(){
                            @Override
                            public void run(){
                                try {
                                    qosScrollPane.setViewportView(getQosTable());
                                    setStatus("QoS pane initialized", false, false);
                                    setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                                    getQoSButton.setEnabled(true);
                                    setQoSButton.setEnabled(true);
                                } catch (CommonException e) {
                                    closed = true;
                                    fireFrameChanged("entity_freed");
                                    dispose();
                                }
                            }
                        };
                        SwingUtilities.invokeLater(worker);
                    }
                } else if(selected.equals(statisticsPane)){
                    refreshItem.setEnabled(true);
                    
                    if(statisticsTable == null){
                        setCursor(new Cursor(Cursor.WAIT_CURSOR));
                        setStatus("Initializing Statistics pane...", true, true);
                        
                        Runnable worker = new Runnable(){
                            @Override
                            public void run(){
                                try {
                                    statisticsPane.setViewportView(getStatisticsTable());
                                    setStatus("Statistics pane initialized", false, false);
                                    setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                                } catch (CommonException e) {
                                    closed = true;
                                    fireFrameChanged("entity_freed");
                                    dispose();
                                }
                            }
                        };
                        SwingUtilities.invokeLater(worker);
                    }
                } else { /*attributes pane.*/
                    refreshItem.setEnabled(true);
                }
            } else if("window_iconified".equals(msg)){
                iconified = true;
            } else if("window_deiconified".equals(msg)){
                iconified = false;
                
                if(updateTimer != null){
                    if(!updateTimer.isRunning()){
                        updateTimer.start();
                    }
                }
            } else if("window_closed".equals(msg)){
                closed = true;
            } else if("close_window".equals(msg)){
                closed = true;
                this.dispose();
            } else if("get_qos".equals(msg)){
                getQoSButton.setEnabled(false);
                setQoSButton.setEnabled(false);
                
                setCursor(new Cursor(Cursor.WAIT_CURSOR));
                setStatus("Resolving QoS...", true, true);
                
                Runnable worker = new Runnable(){
                    @Override
                    public void run(){
                        boolean b = qosTable.update();
                        
                        if(b){
                            setStatus("QoS resolved", false, false);
                            setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                            getQoSButton.setEnabled(true);
                            setQoSButton.setEnabled(true);
                        } else {
                            closed = true;
                            fireFrameChanged("entity_freed");
                            dispose();
                        }
                    }
                };
                SwingUtilities.invokeLater(worker);
            } else if("set_qos".equals(msg)){
                getQoSButton.setEnabled(false);
                setQoSButton.setEnabled(false);
                
                setCursor(new Cursor(Cursor.WAIT_CURSOR));
                setStatus("Applying new QoS...", true, true);
                
                Runnable worker = new Runnable(){
                    @Override
                    public void run(){
                        try {
                            qosTable.applyCurrentQoS();
                            setStatus("New QoS applied.", false, false);
                            setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                            getQoSButton.setEnabled(true);
                            setQoSButton.setEnabled(true);
                        } catch (CommonException e) {
                            setStatus("Error:" + e.getMessage(), false, false);
                            setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                            getQoSButton.setEnabled(true);
                            setQoSButton.setEnabled(true);
                            
                            
                            /*
                            closed = true;
                            fireFrameChanged("entity_freed");
                            dispose();
                            */
                        }
                    }
                };
                SwingUtilities.invokeLater(worker);
            } else if(msg.startsWith("view_")){
                final String command = msg.substring(5);
                setStatus("Changing data type representation to " + type + "...", true, true);
                
                Runnable worker = new Runnable(){
                    @Override
                    public void run(){
                        setDataTypeContentType(command);
                        setStatus("Data type representation changed to " + type, false, false);
                    }
                };
                SwingUtilities.invokeLater(worker);
            } else if("refresh".equals(msg)){
                Component selected = tabbedPane.getSelectedComponent();
                
                if(selected.equals(statusScrollPane)){
                    refreshItem.setEnabled(false);
                    setCursor(new Cursor(Cursor.WAIT_CURSOR));
                    setStatus("Resolving Status...", true, true);
                    
                    Runnable worker = new Runnable(){
                        @Override
                        public void run(){
                            boolean success = ((EntityStatusTableModel)statusTable.getModel()).update();
                            
                            if(success){
                                setStatus("Status resolved.", false, false);
                                setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                                refreshItem.setEnabled(true);
                            } else {
                                setStatus("Error: Status could not be resolved.", false, false);
                                closed = true;
                                fireFrameChanged("entity_freed");
                                dispose();
                            }
                        }
                    };
                    SwingUtilities.invokeLater(worker);
                } else if(selected.equals(statisticsPane)){
                    refreshItem.setEnabled(false);
                    setCursor(new Cursor(Cursor.WAIT_CURSOR));
                    setStatus("Resolving Statistics...", true, true);
                    
                    Runnable worker = new Runnable(){
                        @Override
                        public void run(){
                            boolean success = statisticsTable.update();
                            
                            if(success){
                                setStatus("Statistics resolved.", false, false);
                                setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                                refreshItem.setEnabled(true);
                            } else {
                                setStatus("Error: Statistics could not be resolved.", false, false);
                                closed = true;
                                fireFrameChanged("entity_freed");
                                dispose();
                            }
                        }
                    };
                    SwingUtilities.invokeLater(worker);
                } else if(selected.equals(qosPanel)){
                    this.update("get_qos");
                } else if(selected.equals(attributesScrollPane)){
                    refreshItem.setEnabled(false);
                    setCursor(new Cursor(Cursor.WAIT_CURSOR));
                    setStatus("Refreshing attributes...", true, true);
                    
                    Runnable worker = new Runnable(){
                        @Override
                        public void run(){
                            boolean success = ((EntityAttributeTableModel)(getAttributeTable().getModel())).update();
                            
                            if(success){
                                setStatus("Attributes refreshed.", false, false);
                                setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                                refreshItem.setEnabled(true);
                            } else {
                                setStatus("Error: Attributes could not be refreshed.", false, false);
                                closed = true;
                                fireFrameChanged("entity_freed");
                                dispose();
                            }
                        }
                    };
                    SwingUtilities.invokeLater(worker);
                }
            } else if("entity_freed".equals(msg)){
                closed = true;
                fireFrameChanged("entity_freed");
                dispose();
            } else if ("reset_failed".equals(msg)) {
                setStatus("Warning: Failed to reset field", false, false);
            } else if ("reset_all_failed".equals(msg)) {
                setStatus("Warning: Failed to reset all field", false, false);
            }
        }
    }
    
    /**
     * Sets the supplied status in the status bar.
     * 
     * @param msg The message to display.
     * @param persistent Whether or not the message should be displayed until
     *                   another message is set, or should be removed after an
     *                   amount of timer
     * @param busy Whether or not the status bar should display the progressbar
     *             and set the progress to indeterminate mode. 
     */
    public void setStatus(String msg, boolean persistent, boolean busy){
        statusPanel.setStatus(msg, persistent, busy);
    }
    
    @Override
    public String toString(){
        String result = this.getTitle();
        
        return result;
    }
        
    /**
     * Notifies the ModelRegister about changes in the frame.
     * 
     * @param description The description about what changed.
     */
    protected void fireFrameChanged(String description){
        if((register != null) && (description != null)){
            register.pushUpdate("entity_frame_" + description);
        }
    }
    
	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize() {
		this.setSize(500,300);
        this.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        controller = new EntityInfoListener(this);
        this.addWindowListener(controller);
		this.setContentPane(getRootPanel());
        this.setJMenuBar(getInfoMenuBar());
	}
	/**
	 * This method initializes rootPanel
	 * 
	 * @return The root panel of the frame.
	 */
	private JPanel getRootPanel() {
		if(rootPanel == null) {
			rootPanel = new javax.swing.JPanel();
			rootPanel.setLayout(new java.awt.BorderLayout());
			rootPanel.add(getTabbedPane(), java.awt.BorderLayout.CENTER);
            rootPanel.add(getStatusPanel(), java.awt.BorderLayout.SOUTH);
		}
		return rootPanel;
	}

	/**
	 * This method initializes tabbedPane	
	 * 	
	 * @return The tabbed pane that holds all views on the attached Entity.	
	 */    
	private JTabbedPane getTabbedPane() {
		if (tabbedPane == null) {
			tabbedPane = new JTabbedPane();
			tabbedPane.addTab("Attributes", null, getAttributesScrollPane(), "Entity attributes");
            
            if( (entity instanceof Topic) ||
                (entity instanceof Subscriber) ||
                (entity instanceof DataReader) ||
                (entity instanceof Queue) ||
                (entity instanceof Writer) ||
                (entity instanceof Partition))
            {
                tabbedPane.addTab("Status", null, getStatusScrollPane(), "Entity status");
            }
            if( ((entity instanceof Topic) ||
                (entity instanceof Subscriber) ||
                (entity instanceof Participant) ||
                (entity instanceof Publisher) ||
                (entity instanceof Writer) ||
                (entity instanceof Reader)) &&
                (!(entity instanceof Query)))
            {
                tabbedPane.addTab("QoS", null, getQosPanel(), "Entity QoS policies");
            }
            
            if( (entity instanceof Topic) ||
                (entity instanceof DataReader) ||
                (entity instanceof Queue) || 
                (entity instanceof Query) ||
                (entity instanceof Writer))
            {
                tabbedPane.addTab("Data type", null, getDataTypeScrollPane(), "Data type");
            }
            
            tabbedPane.addTab("Statistics", null, getStatisticsPane(), "Entity statistics");
            
            tabbedPane.addChangeListener(controller);
		}
		return tabbedPane;
	}
    
    /**
     * This method intializes dataTypePane. It is only displayed for Topic
     * entities and displays the data type of the Topic.
     * 
     * @return The EntityInfoPane that displays the data type of the Topic. 
     */
    private EntityInfoPane getDataTypePane(){
        if (dataTypePane == null){
            
            dataTypePane = new EntityInfoPane(type);
            
            try {
                if(entity instanceof Topic){
                    dataTypePane.setSelection(((Topic)entity).getDataType());
                } else if(entity instanceof Reader){
                    dataTypePane.setSelection(((Reader)entity).getDataType());
                }  else if(entity instanceof Writer){
                    dataTypePane.setSelection(((Writer)entity).getDataType());
                }
            } catch (CMException e) {
                if(updateTimer != null){
                    if(updateTimer.isRunning()){
                        updateTimer.stop();
                    }
                    updateTimer = null;
                }
                this.fireFrameChanged("entity_freed");
                this.dispose();
            } catch (DataTypeUnsupportedException de) {
                dataTypePane.setSelection("Data type not supported.");
            }
        }
        return dataTypePane;
    }
    
	/**
	 * This method initializes attributesScrollPane	
	 * 	
	 * @return The pane that holds the attributesTable.	
	 */    
	private JScrollPane getAttributesScrollPane() {
		if (attributesScrollPane == null) {
			attributesScrollPane = new JScrollPane();
			attributesScrollPane.setViewportView(getAttributeTable());
		}
		return attributesScrollPane;
	}
    
	/**
	 * This method initializes statusScrollPane	
	 * 
	 * @return Pane that holds the statusTable.	
	 */    
	private JScrollPane getStatusScrollPane() {
		if (statusScrollPane == null) {
			statusScrollPane = new JScrollPane();
			//statusScrollPane.setViewportView(getStatusTable());
		}
		return statusScrollPane;
	}
    
	/**
	 * This method initializes statusTable	
	 * 	
	 * @return Table that holds the current status of the attached Entity.	
	 * @throws CommonException
	 */    
	private JTable getStatusTable() throws CommonException {
		if (statusTable == null) {
            EntityStatusTableModel model = null;
		    
            if(entity instanceof Partition){
                model = new PartitionStatusTableModel((Partition)entity);
            } else if(entity instanceof Topic){
                model = new TopicStatusTableModel((Topic)entity);
            } else if(entity instanceof Subscriber){
                model = new SubscriberStatusTableModel((Subscriber)entity);
            } else if(entity instanceof Writer){
                model = new WriterStatusTableModel((Writer)entity);
            } else if(entity instanceof DataReader || entity instanceof Queue){
                model = new ReaderStatusTableModel((Reader)entity);
            }
            statusTable = new JTable(model);
            statusTable.setPreferredScrollableViewportSize(new java.awt.Dimension(300,150));
            statusTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
            statusTable.getTableHeader().setReorderingAllowed(false);
            TableColumnModel tcm = statusTable.getColumnModel();
            tcm.getColumn(0).setPreferredWidth(210);
            tcm.getColumn(0).setMaxWidth(210);
            tcm.getColumn(1).setPreferredWidth(145);
            tcm.getColumn(1).setMaxWidth(145);
		}
		return statusTable;
	}
    
	/**
	 * This method initializes qosPanel	
	 * 	
	 * @return Currently an empty panel.	
	 */    
	private JPanel getQosPanel() {
		if (qosPanel == null) {
			qosPanel = new JPanel();
            qosPanel.setLayout(new BorderLayout());
            qosPanel.add(getQosScrollPane(), BorderLayout.CENTER);
            qosPanel.add(getButtonPanel(), BorderLayout.SOUTH);
		}
		return qosPanel;
	}
    
	/**
	 * This method initializes dataTypeScrollPane	
	 * 	
	 * @return Pane that holds the dataTypePane.	
	 */    
	private JScrollPane getDataTypeScrollPane() {
		if (dataTypeScrollPane == null) {
			dataTypeScrollPane = new JScrollPane();
			//dataTypeScrollPane.setViewportView(getDataTypePane());
		}
		return dataTypeScrollPane;
	}
    
    private StatusPanel getStatusPanel(){
        if(statusPanel == null){
            statusPanel = new StatusPanel(this.getWidth(), "", false, true);
        }
        return statusPanel;
    }
    
    /**
     * Provides access to infoMenuBar.
     * 
     * @return Returns the infoMenuBar.
     */
    private JMenuBar getInfoMenuBar() {
        if(infoMenuBar == null){
            infoMenuBar = new JMenuBar();
            infoMenuBar.add(getFileMenu());
            infoMenuBar.add(getEditMenu());
            infoMenuBar.add(getViewMenu());
        }
        return infoMenuBar;
    }
    
    private JMenu getFileMenu() {
        if(fileMenu == null) {
            fileMenu = new JMenu();
            fileMenu.setText("File");
            fileMenu.setMnemonic(java.awt.event.KeyEvent.VK_F);
            
            fileMenu.add(getCloseItem());
        }
        return fileMenu;
    }
    
    private JMenu getEditMenu() {
        if(editMenu == null) {
            editMenu = new JMenu();
            editMenu.setText("Edit");
            editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
            
            editMenu.add(this.getRefreshItem());
        }
        return editMenu;
    }
    
    private JMenu getViewMenu() {
        if(viewMenu == null) {
            viewMenu = new JMenu();
            viewMenu.setText("View");
            viewMenu.setMnemonic(java.awt.event.KeyEvent.VK_V);
            
            if( (entity instanceof Topic) ||
                (entity instanceof DataReader) ||
                (entity instanceof Queue) || 
                (entity instanceof Query) ||
                (entity instanceof Writer))
            {
                ButtonGroup group = new ButtonGroup();
                group.add(getContentHtmlItem());
                group.add(getContentTextItem());
                
                JMenu viewDataTypeMenu = new JMenu();
                viewDataTypeMenu.setText("Data type representation");
                viewDataTypeMenu.add(getContentTextItem());
                viewDataTypeMenu.add(getContentHtmlItem());
                viewMenu.add(viewDataTypeMenu);
                selectContentType();
                
            }
            
            if(parent != null){
                viewMenu.add(getViewMainItem());
            }
        }
        return viewMenu;
    }
    
    private JMenuItem getViewMainItem(){
        if(viewMainItem == null){
            viewMainItem = new JMenuItem();
            viewMainItem.setText("Show main window");
            viewMainItem.setMnemonic(java.awt.event.KeyEvent.VK_S);
            viewMainItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_S, java.awt.Event.SHIFT_MASK, false));
            viewMainItem.setActionCommand("view_main");
            viewMainItem.addActionListener(new MainWindowOpener(parent));
        }
        return viewMainItem;
    }
    /**
     * Provides access to closeItem.
     * 
     * @return Returns the closeItem.
     */
    private JMenuItem getCloseItem() {
        if(closeItem == null){
            closeItem = new JMenuItem();
            closeItem.setActionCommand("close");
            closeItem.setMnemonic(KeyEvent.VK_C);
            closeItem.setText("Close");
            closeItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_C, Event.CTRL_MASK, false));
            closeItem.addActionListener(controller);
        }
        return closeItem;
    }
    
    /**
     * Provides access to refreshStatusItem.
     * 
     * @return Returns the refreshStatusItem.
     */
    private JMenuItem getRefreshItem() {
        if(refreshItem == null){
            refreshItem = new JMenuItem();
            refreshItem.setText("Refresh");
            refreshItem.setActionCommand("refresh");
            refreshItem.setMnemonic(KeyEvent.VK_R);
            refreshItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F5, 0, false));
            refreshItem.addActionListener(controller);
        }
        return refreshItem;
    }
    
    /**
     * Provides access to buttonPanel.
     * 
     * @return Returns the buttonPanel.
     */
    private JPanel getButtonPanel() {
        if(buttonPanel == null){
            Dimension d = new Dimension(200, 20);
            buttonPanel = new JPanel();
            buttonPanel.setLayout(new BorderLayout());
            buttonPanel.setPreferredSize(d);
            buttonPanel.setMaximumSize(d);
            buttonPanel.add(getGetQoSButton(), BorderLayout.WEST);
            buttonPanel.add(getSetQoSButton(), BorderLayout.EAST);
        }
        return buttonPanel;
    }
    /**
     * Provides access to getQoSButton.
     * 
     * @return Returns the getQoSButton.
     */
    private JButton getGetQoSButton() {
        if(getQoSButton == null){
            Dimension d = new Dimension(100, 20);
            getQoSButton = new JButton();
            getQoSButton.setText("Get");
            getQoSButton.setToolTipText("Resolves the current QoS policies of '" + entity + "'");
            getQoSButton.setPreferredSize(d);
            getQoSButton.setMaximumSize(d);
            getQoSButton.setMinimumSize(d);
            getQoSButton.setEnabled(false);
            getQoSButton.setActionCommand("get_qos");
            getQoSButton.addActionListener(controller);
        }
        return getQoSButton;
    }
    /**
     * Provides access to setQoSButton.
     * 
     * @return Returns the setQoSButton.
     */
    private JButton getSetQoSButton() {
        if(setQoSButton == null){
            Dimension d = new Dimension(100, 20);
            setQoSButton = new JButton();
            setQoSButton.setText("Set");
            setQoSButton.setToolTipText("Sets the QoS policies of '" + entity + "' to the supplied value.");
            setQoSButton.setPreferredSize(d);
            setQoSButton.setMaximumSize(d);
            setQoSButton.setMinimumSize(d);
            setQoSButton.setEnabled(false);
            setQoSButton.setActionCommand("set_qos");
            setQoSButton.addActionListener(controller);
        }
        return setQoSButton;
    }
 
    private JScrollPane getQosScrollPane(){
        if(qosScrollPane == null){
            qosScrollPane = new JScrollPane();
        }
        return qosScrollPane;
    }
    
    /**
     * Provides access to qosTable.
     * 
     * @return Returns the qosTable.
     * @throws CommonException
     */
    private QoSTable getQosTable() throws CommonException {
        if(qosTable == null){
            qosTable = new QoSTable(entity, statusPanel);
        }
        return qosTable;
    }
    
    private JScrollPane getStatisticsPane() {
        if(statisticsPane == null){
            statisticsPane = new JScrollPane();
        }
        return statisticsPane;
    }
    
    private StatisticsTable getStatisticsTable() throws CommonException {
        if(statisticsTable == null){
            statisticsTable = new StatisticsTable(entity, this);
        }
        return statisticsTable;
    }
    
    /**
     * Provides access to contentHtmlItem.
     * 
     * @return Returns the contentHtmlItem.
     */
    private JRadioButtonMenuItem getContentHtmlItem() {
        if(contentHtmlItem == null){
            contentHtmlItem = new JRadioButtonMenuItem();
            contentHtmlItem.setText("text/html");
            contentHtmlItem.setMnemonic(java.awt.event.KeyEvent.VK_H);
            contentHtmlItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_H, java.awt.Event.ALT_MASK, false));
            contentHtmlItem.setActionCommand("view_text/html");
            contentHtmlItem.addActionListener(controller);
        }
        return contentHtmlItem;
    }
    /**
     * Provides access to contentTextItem.
     * 
     * @return Returns the contentTextItem.
     */
    private JRadioButtonMenuItem getContentTextItem() {
        if(contentTextItem == null){
            contentTextItem = new JRadioButtonMenuItem();
            contentTextItem.setText("text/plain");
            contentTextItem.setMnemonic(java.awt.event.KeyEvent.VK_P);
            contentTextItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_P, java.awt.Event.ALT_MASK, false));
            contentTextItem.setActionCommand("view_text/plain");
            contentTextItem.addActionListener(controller);
        }
        return contentTextItem;
    }
    
    private void selectContentType(){
        if(type == null){
            type = Config.getInstance().getProperty("data_type_content_type");
        }
        
        if("text/plain".equals(type)){
            contentTextItem.setSelected(true);
            contentHtmlItem.setSelected(false);
        } else if("text/html".equals(type)){
            contentTextItem.setSelected(false);
            contentHtmlItem.setSelected(true);
        }
    }
}
