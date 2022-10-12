using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace ActiveRagdoll
{
    public class Gripper : MonoBehaviour
    {
        public GripModule GripMod { get; set; }

        private Rigidbody _lastCollition;
        private Transform _ragdollTransform;
        public Transform RagdollTransform { get { return _ragdollTransform; } set { _ragdollTransform = value; } }

        private FixedJoint _joint;

        private int _playerID;
        private float _defaultGravity;
        public int PlayerID { get { return _playerID; } set { _playerID = value; } }

        private void Awake()
        {
            enabled = false;
        }

        private void Grip(Rigidbody whatToGrip)
        {
 
            if (GripMod.IsGrabbing) //If Other Gripper already is grabbing something
            {
                return;
            }

            if (_joint != null) //If we're already grabbing something
            {
                return;
            }

            if (!enabled) //If we're not actively grabbing
            {
                _lastCollition = whatToGrip;
                return;
            }


            if (whatToGrip.gameObject.GetComponent<DontDestroy>() && whatToGrip.transform.root.GetChild(0).GetComponent<PlayerMovementBehaviour>().PlayerState == PlayerStates.GRABBED) //If we're about to grip a player that is already grabbed by by someone else
            {
                return;
            }

            else
            {
                GripMod.IsGrabbing = true;
            }

            _joint = gameObject.AddComponent<FixedJoint>();
            _joint.connectedBody = whatToGrip;

            var connectedBodyRoot = _joint.connectedBody.transform.root;
            var thisRoot = transform.root;

            if (connectedBodyRoot.GetComponent<ActiveRagdoll>()) //if we grabbed a Player
            {

                _defaultGravity = connectedBodyRoot.GetComponentInChildren<PlayerMovementBehaviour>().gravityScale;
                connectedBodyRoot.GetChild(0).GetComponent<HealthSystem>().LastHitByID = _playerID;
                connectedBodyRoot.GetChild(0).GetComponent<PlayerMovementBehaviour>().UpdateGrabbedState(true, _playerID); //Set grabbed player's state to Grabbed
                connectedBodyRoot.GetChild(0).GetComponent<PlayerMovementBehaviour>().gravityScale = -0.1f;

                connectedBodyRoot.GetComponentInChildren<ConfigurableJoint>().angularXMotion = ConfigurableJointMotion.Free;
                connectedBodyRoot.GetComponentInChildren<ConfigurableJoint>().angularYMotion = ConfigurableJointMotion.Free;
                connectedBodyRoot.GetComponentInChildren<ConfigurableJoint>().angularZMotion = ConfigurableJointMotion.Free;

                connectedBodyRoot.GetChild(0).GetComponent<PlayerMovementBehaviour>().RequiredButtonPresses = thisRoot.GetChild(0).GetComponent<PlayerController>().GripGloves ? 15 : 10;
            }
            else if (_joint.connectedBody.GetComponent<Grippable>())
                _joint.connectedBody.GetComponent<Grippable>().AddToList(_ragdollTransform);
            

        } //Grip()

        private void UnGrip()
        {
            if (_joint == null)
                return;


            if (_joint.connectedBody)
            {
                var connectedBodyRoot = _joint.connectedBody.transform.root;
             

                if (connectedBodyRoot.GetComponent<ActiveRagdoll>()) //if we grabbed a Player
                {
                    connectedBodyRoot.GetChild(0).GetComponent<PlayerMovementBehaviour>().UpdateGrabbedState(false, _playerID); //Ungrip Player
                    connectedBodyRoot.GetChild(0).GetComponent<PlayerMovementBehaviour>().gravityScale = _defaultGravity;

                    connectedBodyRoot.GetComponentInChildren<ConfigurableJoint>().angularXMotion = ConfigurableJointMotion.Limited;
                    connectedBodyRoot.GetComponentInChildren<ConfigurableJoint>().angularYMotion = ConfigurableJointMotion.Limited;
                    connectedBodyRoot.GetComponentInChildren<ConfigurableJoint>().angularZMotion = ConfigurableJointMotion.Limited;
                    
                }
                else if (_joint.connectedBody.GetComponent<Grippable>()) //if we gripped anything EXCEPT a player
                {
                    _joint.connectedBody.GetComponent<Grippable>().RemoveFromList(_ragdollTransform);
                    _joint.connectedBody.AddForce(_ragdollTransform.forward * 20, ForceMode.Impulse);
                }
            }
            Destroy(_joint);

            _joint = null;
            GripMod.IsGrabbing = false;
            gameObject.GetComponent<BoxCollider>().enabled = true;
        } //UnGrip()

        private void OnCollisionEnter(Collision collision)
        {
            if (collision.rigidbody)
                Grip(collision.rigidbody);
        }

        private void OnCollisionExit(Collision collision)
        {
            if (collision.rigidbody == _lastCollition)
            {
                _lastCollition = null;
            }
        }

        private void OnEnable() //When GripModule.GrippEnable() is called
        {
            if (_lastCollition != null)
            {
                 Grip(_lastCollition);
            }
        } //OnEnable()

        private void OnDisable() //When GripModule.GrippDisable() is called
        {             
            UnGrip();
        } //OnDisable()
    } //Gripper class

} //ActiveRagdoll class
