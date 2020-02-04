using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class controllerScript : MonoBehaviour {

  // Use this for initialization
  void Start () {

  }
  // Update is called once per frame
  void Update () {


  }


  public void OnButtonDown(){

    GameObject bullet = Instantiate(Resources.Load("bullet", typeof(GameObject))) as GameObject;
    Rigidbody rb = bullet.GetComponent<Rigidbody>();
    bullet.transform.rotation = Camera.main.transform.rotation;
    bullet.transform.position = Camera.main.transform.position;
    rb.AddForce(Camera.main.transform.forward * 500f);
    bullet.GetComponent<AudioSource>().Play ();
    Destroy (bullet, 3);
  }

}//end of file