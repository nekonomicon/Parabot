package su.xash.xash3d;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.content.Intent;
import android.widget.Spinner;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.Button;
import android.widget.TextView;
import android.content.ComponentName;
import android.content.pm.PackageManager;
import android.content.SharedPreferences;
import java.io.File;

import su.xash.xash3d.parabot.R;

public class LauncherActivity extends Activity {
	static EditText cmdArgs;
	static SharedPreferences mPref;
	static Spinner modSpinner;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Build layout
		setContentView(R.layout.activity_launcher);	
		cmdArgs = (EditText)findViewById(R.id.cmdArgs);
		cmdArgs.setSingleLine(true);
		modSpinner= (Spinner)findViewById(R.id.modSpinner);
                modSpinner.setEnabled(true);
		final String[] list = {
                        "Half-Life",
                        "Bubblemod",
                        "Deathmatch Classic",
                        "Opposing Force", // anti-lost_gamer patch(port Opposing Force or die)
                        "Adrenaline Gamer",
                        "They Hunger"
		};
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item, list);
                adapter.setDropDownViewResource(android.R.layout.simple_spinner_item);
                modSpinner.setAdapter(adapter);
		mPref = getSharedPreferences("bot", 0);
		cmdArgs.setText(mPref.getString("argv", "-dev 3 -log"));
		modSpinner.setSelection(mPref.getInt("spinner", 0));
		InstallReceiver.extractPAK(this, false);
	}

	public void startXash(View view)
	{
		String gamedir;
		Intent intent = new Intent();
		intent.setAction("in.celest.xash3d.START");
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		String argv=cmdArgs.getText().toString();
		SharedPreferences.Editor editor = mPref.edit();
		editor.putString("argv", argv);
		editor.putInt("spinner", modSpinner.getSelectedItemPosition());
		editor.commit();
		String fullPath = getFilesDir().getAbsolutePath().replace("/files", "/lib");
		File pb_hardfp = new File(fullPath + "/libparabot_hardfp.so");
		File pb = new File(fullPath + "/libparabot.so");
		if(pb_hardfp.exists() && !pb_hardfp.isDirectory())
			argv = argv + " -dll " + pb_hardfp.getAbsolutePath();
		else if(pb.exists() && !pb.isDirectory())
			argv = argv + " -dll " + pb.getAbsolutePath();
		fullPath = fullPath.replace("su.xash", "in.celest");
		if(modSpinner.getSelectedItemPosition() == 5)
		{
			fullPath = fullPath.replace("parabot", "hunger");
			gamedir = "Hunger";
			if(!checkLibraryExistence(fullPath, "They Hunger "))
				return;
		}
		else if(modSpinner.getSelectedItemPosition() == 4)
		{
			fullPath = fullPath.replace("in.celest", "su.xash");
			fullPath = fullPath.replace("parabot", "ag");
			gamedir = "ag";
			if(!checkLibraryExistence(fullPath, "Adrenaline Gamer "))
				return;
		}
		else if(modSpinner.getSelectedItemPosition() == 3) // anti-lost_gamer patch(port Opposing Force or die)
		{
			fullPath = fullPath.replace("parabot", "gearbox");
			gamedir = "gearbox";
			if(!checkLibraryExistence(fullPath, "OP4CELauncher "))
				return;
		}
		else if(modSpinner.getSelectedItemPosition() == 2)
		{
			fullPath = fullPath.replace("in.celest", "su.xash");
			fullPath = fullPath.replace("parabot", "dmc");
			gamedir = "dmc";
			if(!checkLibraryExistence(fullPath, "QCLauncher "))
				return;
		}
		else if(modSpinner.getSelectedItemPosition() == 1)
		{
			fullPath = fullPath.replace("parabot", "bubblemod");
			gamedir = "valve";
			if(!checkLibraryExistence(fullPath, "Bubblemod "))
				return;
		}
		else
		{
			fullPath = fullPath.replace("parabot", "hl");
			gamedir = "valve";
		}

		if(cmdArgs.length() != 0) intent.putExtra("argv", argv);
		// Uncomment to set gamedir here
		intent.putExtra("gamedir", gamedir);
		intent.putExtra("gamelibdir", fullPath);
		String[] env = new String[2];
		env[0] = "PARABOT_EXTRAS_PAK";
		env[1] = getFilesDir().getAbsolutePath() + "/extras.pak";
		intent.putExtra("env", env);

		PackageManager pm = getPackageManager();
		if(intent.resolveActivity(pm) != null)
		{
			startActivity(intent);
		}
		else
		{
			showXashInstallDialog("Xash3D FWGS ");
		}
	}

	public void showXashInstallDialog(String msg)
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(this);

		builder.setTitle("Xash error")
		.setMessage(msg + getString(R.string.alert_dialog_text))
		.show();
	}

	public boolean checkLibraryExistence(String Path, String msg)
	{
		File serverlib_hardfp = new File(Path + "/libserver_hardfp.so");
		File serverlib = new File(Path + "/libserver.so");
		if(!serverlib.exists() && !serverlib_hardfp.exists())
		{
			showXashInstallDialog(msg);
			return false;
		}
		return true;
	}
}
